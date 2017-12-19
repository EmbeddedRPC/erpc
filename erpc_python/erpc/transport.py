#!/usr/bin/env python

# The Clear BSD License
# Copyright (c) 2015-2016 Freescale Semiconductor, Inc.
# Copyright 2016-2017 NXP
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted (subject to the limitations in the disclaimer below) provided
# that the following conditions are met:
#
# o Redistributions of source code must retain the above copyright notice, this list
#   of conditions and the following disclaimer.
#
# o Redistributions in binary form must reproduce the above copyright notice, this
#   list of conditions and the following disclaimer in the documentation and/or
#   other materials provided with the distribution.
#
# o Neither the name of the copyright holder nor the names of its
#   contributors may be used to endorse or promote products derived from this
#   software without specific prior written permission.
#
# NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import struct
import serial
import socket
import threading
from .crc16 import Crc16
from .client import RequestError
try:
    from rpmsg.sysfs import RpmsgEndpoint
    RpmsgEndpointReady = True
except ImportError:
    RpmsgEndpointReady = False

##
# @brief Base transport class.
class Transport(object):
    def __init__(self):
        pass

    def send(self, message):
        raise NotImplementedError()

    def receive(self):
        raise NotImplementedError()

class FramedTransport(Transport):
    HEADER_LEN = 4

    def __init__(self):
        super(FramedTransport, self).__init__()
        self._sendLock = threading.Lock()
        self._receiveLock = threading.Lock()
        self._Crc16 = Crc16()

    @property
    def crc_16(self):
        return self._Crc16

    @crc_16.setter
    def crc_16(self, crcStart):
        if type(crcStart) is not int:
            raise RequestError("invalid CRC, not a number")
        self._Crc16 = Crc16(crcStart)

    def send(self, message):
        try:
            self._sendLock.acquire()

            crc = self._Crc16.computeCRC16(message)

            header = bytearray(struct.pack('<HH', len(message), crc))
            assert len(header) == self.HEADER_LEN
            self._base_send(header + message)
        finally:
            self._sendLock.release()

    def receive(self):
        try:
            self._receiveLock.acquire()

            # Read fixed size header containing the message length.
            headerData = self._base_receive(self.HEADER_LEN)
            messageLength, crc = struct.unpack('<HH', headerData)

            # Now we know the length, read the rest of the message.
            data = self._base_receive(messageLength)
            computedCrc = self._Crc16.computeCRC16(data)

            if computedCrc != crc:
                raise RequestError("invalid message CRC")
            return data
        finally:
            self._receiveLock.release()

    def _base_send(self, data):
        raise NotImplementedError()

    def _base_receive(self):
        raise NotImplementedError()

class SerialTransport(FramedTransport):
    def __init__(self, url, baudrate, **kwargs):
        super(SerialTransport, self).__init__()
        self._url = url
        self._serial = serial.serial_for_url(url, baudrate=baudrate, **kwargs) # 8N1 by default

    def close(self):
        self._serial.close()

    def _base_send(self, data):
        self._serial.write(data)

    def _base_receive(self, count):
        return self._serial.read(count)

class ConnectionClosed(Exception):
    pass

class TCPTransport(FramedTransport):
    def __init__(self, host, port, isServer):
        super(TCPTransport, self).__init__()
        self._host = host
        self._port = port
        self._isServer = isServer
        self._sock = None

        if self._isServer:
            self._serverThread = threading.Thread(target=self._serve)
            self._serverThread.daemon = True
            self._serverThread.start()
        else:
            self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self._sock.connect((self._host, self._port))

    def _serve(self):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.setblocking(1)
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((self._host, self._port))
        s.listen(5)

        while True:
            self._sock, _ = s.accept()

    def close(self):
        self._sock.close()
        self._sock = None

    def _base_send(self, message):
        if self._isServer and not self._sock:
            while not self._sock:
                pass
        if self._sock:
            self._sock.sendall(message)

    def _base_receive(self, count):
        if self._isServer and not self._sock:
            while not self._sock:
                pass
        if self._sock:
            remaining = count
            result = bytearray()
            while remaining:
                data = self._sock.recv(remaining)
                if len(data) == 0:
                    self.close()
                    raise ConnectionClosed()
                result += data
                remaining -= len(data)
            return result


class RpmsgTransport(Transport):
    def __init__(self, ept_addr_local = None, ept_addr_remote = None, channel_name = None):
        if not RpmsgEndpointReady:
            print("Please, install RPMsg from: https://github.com/EmbeddedRPC/erpc-imx-demos/tree/master/middleware/rpmsg-python")
            raise ImportError

        if ept_addr_local is None:
            ept_addr_local = RpmsgEndpoint.LOCAL_DEFAULT_ADDRESS
        if ept_addr_remote is None:
            ept_addr_remote = RpmsgEndpoint.REMOTE_DEFAULT_ADDRESS
        if channel_name is None:
            channel_name = RpmsgEndpoint.rpmsg_openamp_channel

        self.ept_addr_remote = ept_addr_remote
        self.ept = RpmsgEndpoint(
            channel_name,
            ept_addr_local,
            RpmsgEndpoint.Types.DATAGRAM)


    def send(self, message):
        self.ept.send(message, self.ept_addr_remote)

    def receive(self):
        while True:
            ret = self.ept.recv(-1)
            if len(ret[1]) != 0:
                return ret[1]
            else:
                time.sleep(0.001)
        return ret[1]

