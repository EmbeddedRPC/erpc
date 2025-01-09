#!/usr/bin/env python

# Copyright (c) 2015-2016 Freescale Semiconductor, Inc.
# Copyright 2016-2025 NXP
# Copyright 2022 ACRIOS Systems s.r.o.
#
# SPDX-License-Identifier: BSD-3-Clause

import socket
import struct
import threading
import time
from typing import Optional, Any, Union

from .client import RequestError
from .crc16 import Crc16


class TransportError(RuntimeError):
    pass


class Transport(object):
    """ Base transport class.
    """

    def __init__(self):
        pass

    def send(self, message: Union[bytes, bytearray]) -> None:
        raise NotImplementedError()

    def receive(self) -> bytearray:
        raise NotImplementedError()


class FramedTransport(Transport):
    HEADER_LEN = 6

    def __init__(self):
        super(FramedTransport, self).__init__()
        self._send_lock = threading.Lock()
        self._receive_lock = threading.Lock()
        self._crc16 = Crc16()

    @property
    def crc_16(self):
        return self._crc16

    @crc_16.setter
    def crc_16(self, crcStart: int):
        if type(crcStart) is not int:
            raise RequestError("invalid CRC, not a number")
        self._crc16 = Crc16(crcStart)

    def send(self, message: Union[bytes, bytearray]):
        try:
            self._send_lock.acquire()

            crc_body = self._crc16.compute_crc16(message)
            message_length = len(message)
            crc_header = self._crc16.compute_crc16(
                bytes(struct.pack('<H', message_length))) + self._crc16.compute_crc16(
                bytes(struct.pack('<H', crc_body)))
            crc_header &= 0xFFFF  # 2bytes

            header = bytes(struct.pack('<HHH', crc_header, message_length, crc_body))
            assert len(header) == self.HEADER_LEN
            self._base_send(header + message)
        finally:
            self._send_lock.release()

    def receive(self) -> bytearray:
        try:
            self._receive_lock.acquire()

            # Read fixed size header containing the message length.
            header_data = self._base_receive(self.HEADER_LEN)
            crc_header, message_length, crc_body = struct.unpack('<HHH', header_data)

            computed_crc = self._crc16.compute_crc16(
                bytes(struct.pack('<H', message_length))) + self._crc16.compute_crc16(
                bytes(struct.pack('<H', crc_body)))
            computed_crc &= 0xFFFF  # 2bytes

            if computed_crc != crc_header:
                raise RequestError("invalid header CRC")

            # Now we know the length, read the rest of the message.
            data = bytearray(self._base_receive(message_length))
            computed_crc = self._crc16.compute_crc16(data)
            if computed_crc != crc_body:
                raise RequestError("invalid message CRC")

            return data
        finally:
            self._receive_lock.release()

    def _base_send(self, data: Union[bytes, bytearray]) -> None:
        raise NotImplementedError()

    def _base_receive(self, count: int) -> bytes:
        raise NotImplementedError()


class SerialTransport(FramedTransport):
    def __init__(self, url: str, baudrate: int, **kwargs: Any):
        super(SerialTransport, self).__init__()

        try:
            import serial
        except ImportError:
            raise ImportError(
                "Please, install pySerial module (sudo pip3 install pyserial).")

        self._url = url
        self._serial = serial.serial_for_url(
            url, baudrate=baudrate, **kwargs)  # 8N1 by default

    def close(self):
        self._serial.close()

    def _base_send(self, data: bytes):
        self._serial.write(data)

    def _base_receive(self, count: int) -> bytes:
        return self._serial.read(count)


class ConnectionClosed(Exception):
    pass


class TCPTransport(FramedTransport):
    def __init__(self, host: str, port: int, isServer: bool):
        super(TCPTransport, self).__init__()
        self._host = host
        self._port = port
        self._is_server = isServer
        self._sock: Optional[socket.socket] = None
        self._socket_lock = threading.Lock()

        if self._is_server:
            self._server_thread = threading.Thread(target=self._serve)
            self._server_thread.daemon = True
            self._server_thread.start()
            self._server_sock_event_start = threading.Event()
        else:
            self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self._sock.setsockopt(socket.SOL_TCP, socket.TCP_NODELAY, 1)
            self._sock.connect((self._host, self._port))

    def _serve(self):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.setblocking(True)
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.setsockopt(socket.SOL_TCP, socket.TCP_NODELAY, 1)
        s.bind((self._host, self._port))
        s.listen(5)

        while True:
            self._sock, _ = s.accept()
            self._server_sock_event_start.set()

    def close(self):
        if self._is_server:
            self._server_sock_event_start.clear()

        with self._socket_lock:
            if self._sock is not None:
                try:
                    self._sock.shutdown(socket.SHUT_RDWR)
                    self._sock.close()
                except OSError:
                    # May be raised by the OS if the socket was closed externally,
                    # thus invalidating the file descriptor.
                    pass
                self._sock = None

    def _base_send(self, data: bytes):
        if self._is_server:
            self._server_sock_event_start.wait()
        if self._sock:
            self._sock.sendall(data)

    def _base_receive(self, count: int) -> bytes:
        if self._is_server:
            self._server_sock_event_start.wait()

        if self._sock:
            remaining = count
            result = bytes()
            while remaining:
                data = self._sock.recv(remaining)
                if len(data) == 0:
                    self.close()
                    raise ConnectionClosed()
                result += data
                remaining -= len(data)
            return result

        return bytes()




class RpmsgTransport(Transport):
    def __init__(self, ept_addr_local: Optional[int] = None, ept_addr_remote: Optional[int] = None,
                 channel_name: Optional[str] = None):
        super().__init__()

        try:
            from rpmsg.sysfs import RpmsgEndpoint
        except ImportError:
            raise ImportError(
                "Please, install RPMsg from: https://github.com/EmbeddedRPC/erpc-imx-demos/tree/master/middleware/rpmsg-python")

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

    def send(self, message: Union[bytes, bytearray]) -> None:
        self.ept.send(message, self.ept_addr_remote)

    def receive(self) -> bytearray:
        while True:
            ret: Any = self.ept.recv(-1)  # Deal with untyped rpmsg
            if len(ret[1]) != 0:
                return ret[1]
            else:
                time.sleep(0.001)


class LIBUSBSIOSPITransport(FramedTransport):
    def __init__(self, baudrate: Optional[int] = None, cs_gpio_port: Optional[int] = None,
                 cs_gpio_pin: Optional[int] = None, devidx: Optional[int] = None):
        super(LIBUSBSIOSPITransport, self).__init__()

        try:
            from libusbsio import LIBUSBSIO
        except ImportError:
            raise ImportError("Please, install LIBUSBSIO module")

        if baudrate is None:
            baudrate = 500000

        if devidx is None:
            devidx = 0

        self._baudrate = baudrate
        self._cs_gpio_port = cs_gpio_port
        self._cs_gpio_pin = cs_gpio_pin
        self._devidx = devidx
        self._gpio_port = 0
        self._gpio_pin = 0
        self._gpio_mode = 0

        # Load DLL from default directory without any debugging prints
        self.sio = LIBUSBSIO()

        # Get number of LIBUSBSIO devices
        result = self.sio.GetNumPorts(vidpids=[LIBUSBSIO.VIDPID_LPCLINK2])
        if result != 0:
            self._gpio_port = 1
            self._gpio_pin = 2
            self._gpio_mode = 1
        else:
            result = self.sio.GetNumPorts(vidpids=[LIBUSBSIO.VIDPID_MCULINK])
            if result != 0:
                self._gpio_port = 0
                self._gpio_pin = 4
                self._gpio_mode = 0x100
            else:
                raise TransportError("No LIBUSBSIO devices found")

        print(f"Total LIBUSBSIO devices: {result}")

        # Open device at given index
        self._hSIOPort = self.sio.Open(int(self._devidx))

        # Get the device version
        device_version = self.sio.GetVersion()
        print(f"Device version: {device_version}")

        # Get number of available SPI ports
        num_spi_ports = self.sio.GetNumSPIPorts()
        print(f"Number of SPI ports available: {num_spi_ports}")

        # Get max number of bytes supported for I2C/SPI transfers
        max_num_bytes = self.sio.GetMaxDataSize()
        print(f"Max number of bytes supported for I2C/SPI transfers: {max_num_bytes}")

        # Call SPI_Open and store the _hSPIPort handler
        self._hSPIPort = self.sio.SPI_Open(
            int(self._baudrate), portNum=0, dataSize=8, preDelay=0)

        # Configure GPIO pin for SPI master-slave signalling
        result = self.sio.GPIO_ConfigIOPin(
            self._gpio_port, self._gpio_pin, self._gpio_mode)
        print(f"GPIO_ConfigIOPin result: {result}")

        result = self.sio.GPIO_SetPortInDir(self._gpio_port, self._gpio_pin)
        print(f"GPIO_SetPortInDir result: {result}")

        result = self.sio.GPIO_GetPin(self._gpio_port, self._gpio_pin)
        print(f"GPIO_GetPin result: {result}")

    def close(self):
        if self._hSPIPort:
            self._hSPIPort.Close()
            self._hSPIPort = None

        if self.sio:
            self.sio.Close()
            self._hSIOPort = None

    def _base_send(self, data: Union[bytes, bytearray]) -> None:
        assert self._hSPIPort, "No SPI port defined"

        # Wait for SPI master-slave signalling GPIO pin to be in low state
        result = self.sio.GPIO_GetPin(self._gpio_port, self._gpio_pin)
        while 1 == result:
            result = self.sio.GPIO_GetPin(self._gpio_port, self._gpio_pin)

        # Send the header first
        _, rx_bytes_number = self._hSPIPort.Transfer(
            0, 15, data[:self.HEADER_LEN], self.HEADER_LEN, 0)

        if rx_bytes_number <= 0:
            raise TransportError(f"SPI transfer error: {rx_bytes_number}")

        _, rx_bytes_number = self._hSPIPort.Transfer(0, 15, bytes(data[self.HEADER_LEN:]),
                                                     len(data) - self.HEADER_LEN, 0)

        if rx_bytes_number <= 0:
            raise TransportError(f"SPI transfer error: {rx_bytes_number}")

    def _base_receive(self, count: int) -> bytes:
        assert self._hSPIPort, "No SPI port defined"

        # Wait for SPI master-slave signalling GPIO pin to be in low state
        result = self.sio.GPIO_GetPin(self._gpio_port, self._gpio_pin)
        while 1 == result:
            result = self.sio.GPIO_GetPin(self._gpio_port, self._gpio_pin)

        data, rx_bytes_number = self._hSPIPort.Transfer(
            0, 15, bytes(range(count)), count, 0)

        if rx_bytes_number > 0:
            return bytes(data[:count])
        else:
            self._hSPIPort.Reset()
            raise TransportError(f"SPI transfer error: {rx_bytes_number}")


class LIBUSBSIOI2CTransport(FramedTransport):
    def __init__(self, baudrate: Optional[int] = None, devidx: Optional[int] = None):
        super(LIBUSBSIOI2CTransport, self).__init__()

        try:
            from libusbsio import LIBUSBSIO
        except ImportError:
            raise ImportError("Please, install LIBUSBSIO module")

        if baudrate is None:
            baudrate = 100000

        if devidx is None:
            devidx = 0

        self._baudrate = baudrate
        self._devidx = devidx
        self._gpio_port = 0
        self._gpio_pin = 0
        self._gpio_mode = 0

        # Load DLL from default directory without any debugging prints
        self.sio = LIBUSBSIO()

        # Get number of LIBUSBSIO devices
        result = self.sio.GetNumPorts(vidpids=[LIBUSBSIO.VIDPID_LPCLINK2])
        if result != 0:
            self._gpio_port = 1
            self._gpio_pin = 2
            self._gpio_mode = 1
        else:
            result = self.sio.GetNumPorts(vidpids=[LIBUSBSIO.VIDPID_MCULINK])
            if result != 0:
                self._gpio_port = 1
                self._gpio_pin = 3
                self._gpio_mode = 0x100
            else:
                raise TransportError('No LIBUSBSIO devices found')

        print(f"Total LIBUSBSIO devices: {result}")

        # Open device at given index
        self._hSIOPort = self.sio.Open(int(self._devidx))

        # Get the device version
        version = self.sio.GetVersion()
        print(f"Device version: {version}")

        # Get number of available I2C ports
        num_spi_ports = self.sio.GetNumI2CPorts()
        print(f"Number of I2C ports available: {num_spi_ports}")

        # Get max number of bytes supported for SPI/I2C transfers
        max_num_bytes = self.sio.GetMaxDataSize()
        print(f"Max number of bytes supported for SPI/I2C transfers: {max_num_bytes}")

        # Call I2C_Open and store the _hI2CPort handler
        self._hI2CPort = self.sio.I2C_Open(int(self._baudrate), 0, 0)

        # Configure GPIO pin for I2C master-slave signalling
        result = self.sio.GPIO_ConfigIOPin(
            self._gpio_port, self._gpio_pin, self._gpio_mode)
        print(f"GPIO_ConfigIOPin result: {result}")

        result = self.sio.GPIO_SetPortInDir(self._gpio_port, self._gpio_pin)
        print(f"GPIO_SetPortInDir result: {result}")

        result = self.sio.GPIO_GetPin(self._gpio_port, self._gpio_pin)
        print(f"GPIO_GetPin result: {result}")

    def close(self):
        if self._hI2CPort:
            self._hI2CPort.Close()
            self._hI2CPort = None

        if self.sio:
            self.sio.Close()
            self._hI2CPort = None

    def _base_send(self, data: Union[bytes, bytearray]) -> None:
        assert self._hI2CPort, "No I2C port defined"

        # Wait for I2C master-slave signalling GPIO pin to be in low state
        result = self.sio.GPIO_GetPin(self._gpio_port, self._gpio_pin)
        while 1 == result:
            result = self.sio.GPIO_GetPin(self._gpio_port, self._gpio_pin)

        # Send the header first
        _, rx_bytes_number = self._hI2CPort.FastXfer(
            0x7E, data[:self.HEADER_LEN], self.HEADER_LEN, 0, False, True)

        if rx_bytes_number <= 0:
            raise TransportError(f"I2C transfer error: {rx_bytes_number}")

        # Send the payload/data
        _, rx_bytes_number = self._hI2CPort.FastXfer(
            0x7E, bytes(data[self.HEADER_LEN:]), len(data) - self.HEADER_LEN, 0, False, True)

        if rx_bytes_number <= 0:
            raise TransportError(f"I2C transfer error: {rx_bytes_number}")

    def _base_receive(self, count: int) -> bytes:
        assert self._hI2CPort, "No I2C port defined"

        # Wait for I2C master-slave signalling GPIO pin to be in low state
        result = self.sio.GPIO_GetPin(self._gpio_port, self._gpio_pin)
        while 1 == result:
            result = self.sio.GPIO_GetPin(self._gpio_port, self._gpio_pin)

        # Issue the I2C_Transfer API
        data, rx_bytes_number = self._hI2CPort.FastXfer(0x7E, bytes(0), 0, count, False, True)

        if rx_bytes_number > 0:
            return bytes(data[:count])
        else:
            self._hI2CPort.Reset()
            raise TransportError(f"I2C transfer error: {rx_bytes_number}")
