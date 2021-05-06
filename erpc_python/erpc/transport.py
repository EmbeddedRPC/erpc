#!/usr/bin/env python

# Copyright (c) 2015-2016 Freescale Semiconductor, Inc.
# Copyright 2016-2021 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

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

try:
    from libusbsio import *
    LIBUSBSIOReady = True
except ImportError:
    LIBUSBSIOReady = False

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

class LIBUSBSIOSPITransport(FramedTransport):
    def __init__(self, baudrate = None, cs_gpio_port = None, cs_gpio_pin = None):
        super(LIBUSBSIOSPITransport, self).__init__()

        if not LIBUSBSIOReady:
            print("Please, install LIBUSBSIO module")
            raise ImportError

        if baudrate is None:
            baudrate = 500000

        self._baudrate = baudrate
        self._cs_gpio_port = cs_gpio_port
        self._cs_gpio_pin = cs_gpio_pin

        # Load DLL from default directory without any debugging prints
        self.sio = LIBUSBSIO()

        # Get number of LIBUSBSIO devices
        res = self.sio.GetNumPorts ()
        print('Total LIBUSBSIO devices: %d \r\n' % res)

        # Open device at index 0
        self._hSIOPort = self.sio.Open (0)
    
        # Get the device version
        s = self.sio.GetVersion ()
        str1 = ""
        print('Device version: %s \r\n ' % str1.join(str(s)))

        # Get number of available SPI ports
        num_spi_ports = self.sio.GetNumSPIPorts ()
        print('Number of SPI ports available: %d \r\n' % num_spi_ports)

        # Get max number of bytes supported for I2C/SPI transfers
        max_num_bytes = self.sio.GetMaxDataSize ()
        print('Max number of bytes supported for I2C/SPI transfers: %d \r\n' % max_num_bytes)

        # Call SPI_Open and store the _hSPIPort handler 
        self._hSPIPort = self.sio.SPI_Open (int(self._baudrate), portNum=0, dataSize=8, preDelay=0)

        # Configure GPIO pin for SPI master-slave signalling 
        res = self.sio.GPIO_ConfigIOPin (1, 2, 1)
        print('GPIO_ConfigIOPin res: %d \r\n' % res)
        res = self.sio.GPIO_SetPortInDir (1, 2)
        print('GPIO_SetPortInDir res: %d \r\n' % res)
        res = self.sio.GPIO_GetPin (1, 2)
        print('GPIO_GetPin res: %d \r\n' % res)

    def close(self):
        res = self._hSPIPort.Close ()
        self._hSPIPort = None
        res = self.sio.Close ()
        self._hSIOPort = None

    def _base_send(self, message):
        # Wait for SPI master-slave signalling GPIO pin to be in low state 
        res = self.sio.GPIO_GetPin (1, 2)
        while (1 == res):
            res = self.sio.GPIO_GetPin (1, 2)
        # Send the header first
        data, rxbytesnumber = self._hSPIPort.Transfer (0, 15, message[:self.HEADER_LEN], self.HEADER_LEN, 0 )
        if rxbytesnumber>0:
            #print('SPI received %d number of bytes' % rxbytesnumber)
            # Send the payload/data
            data, rxbytesnumber = self._hSPIPort.Transfer (0, 15, bytes(message[4:]), len(message) - self.HEADER_LEN, 0 )
        else:
            print('SPI transfer error: %d' % rxbytesnumber)

    def _base_receive(self, count):
        # Wait for SPI master-slave signalling GPIO pin to be in low state 
        res = self.sio.GPIO_GetPin (1, 2)
        while (1 == res):
            res = self.sio.GPIO_GetPin (1, 2)
        data, rxbytesnumber = self._hSPIPort.Transfer (0, 15, range(count), count, 0 )
        if rxbytesnumber>0:
            #print('SPI received %d number of bytes' % rxbytesnumber)
            return bytes(data[:count])
        else:
            print('SPI transfer error: %d' % rxbytesnumber)
            res = self._hSPIPort.Reset ()
            return b"\00" * count

class LIBUSBSIOI2CTransport(FramedTransport):
    def __init__(self, baudrate = None):
        super(LIBUSBSIOI2CTransport, self).__init__()

        if not LIBUSBSIOReady:
            print("Please, install LIBUSBSIO module")
            raise ImportError

        if baudrate is None:
            baudrate = 100000

        self._baudrate = baudrate

        # Load DLL from default directory without any debugging prints
        self.sio = LIBUSBSIO()

        #Get number of LIBUSBSIO devices
        res = self.sio.GetNumPorts ()
        print('Total LIBUSBSIO devices: %d \r\n' % res)

        # Open device at index 0
        self._hSIOPort = self.sio.Open (0)
    
        # Get the device version
        s = self.sio.GetVersion ()
        str1 = ""
        print('Device version: %s \r\n ' % str1.join(str(s)))

        # Get number of available I2C ports
        num_spi_ports = self.sio.GetNumI2CPorts ()
        print('Number of I2C ports available: %d \r\n' % num_spi_ports)

        # Get max number of bytes supported for SPI/I2C transfers
        max_num_bytes = self.sio.GetMaxDataSize ()
        print('Max number of bytes supported for SPI/I2C transfers: %d \r\n' % max_num_bytes)

        # Call I2C_Open and store the _hI2CPort handler 
        self._hI2CPort = self.sio.I2C_Open (int(self._baudrate), 0, 0)

        # Configure GPIO pin for I2C master-slave signalling 
        res = self.sio.GPIO_ConfigIOPin (1, 2, 1)
        print('GPIO_ConfigIOPin res: %d \r\n' % res)
        res = self.sio.GPIO_SetPortInDir (1, 2)
        print('GPIO_SetPortInDir res: %d \r\n' % res)
        res = self.sio.GPIO_GetPin (1, 2)
        print('GPIO_GetPin res: %d \r\n' % res)

    def close(self):
        res = self._hI2CPort.Close ()
        self._hI2CPort = None
        res = self.sio.Close ()
        self._hSIOPort = None

    def _base_send(self, message):
        # Wait for I2C master-slave signalling GPIO pin to be in low state 
        res = self.sio.GPIO_GetPin (1, 2)
        while (1 == res):
            res = self.sio.GPIO_GetPin (1, 2)
        # Send the header first
        data, rxbytesnumber = self._hI2CPort.FastXfer (0x7E, message[:self.HEADER_LEN], self.HEADER_LEN, 0, 0)
        if rxbytesnumber>0:
            #print('I2C received %d number of bytes' % rxbytesnumber)
            # Send the payload/data
            data, rxbytesnumber = self._hI2CPort.FastXfer (0x7E, bytes(message[4:]), len(message) - self.HEADER_LEN, 0, 0)
        else:
            print('I2C transfer error: %d' % rxbytesnumber)

    def _base_receive(self, count):
        # Wait for I2C master-slave signalling GPIO pin to be in low state 
        res = self.sio.GPIO_GetPin (1, 2)
        while (1 == res):
            res = self.sio.GPIO_GetPin (1, 2)
        # Issue the I2C_Transfer API
        data, rxbytesnumber = self._hI2CPort.FastXfer (0x7E, 0, 0, count, 0)
        if rxbytesnumber>0:
            #print('I2C received %d number of bytes' % rxbytesnumber)
            return bytes(data[:count])
        else:
            #print('I2C transfer error: %d' % rxbytesnumber)
            res = self._hI2CPort.Reset ()
            return b"\00" * count
 