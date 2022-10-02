#!/usr/bin/env python

# Copyright (c) 2015 Freescale Semiconductor, Inc.
# Copyright 2016-2017 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import struct
from .codec import (MessageType, MessageInfo, Codec, CodecError)


class BasicCodec(Codec):
    """Version of this codec.

    Args:
        Codec (code.Codec): Inherit and implement codec interface functions.
    """

    BASIC_CODEC_VERSION = 1

    def start_write_message(self, msgInfo):
        header = (self.BASIC_CODEC_VERSION << 24) \
            | ((msgInfo.service & 0xff) << 16) \
            | ((msgInfo.request & 0xff) << 8) \
            | (msgInfo.type.value & 0xff)
        self.write_uint32(header)
        self.write_uint32(msgInfo.sequence)

    def _write(self, fmt, value):
        self._buffer += struct.pack(fmt, value)
        self._cursor += struct.calcsize(fmt)

    def write_bool(self, value):
        self._write('<?', value)

    def write_int8(self, value):
        self._write('<b', value)

    def write_int16(self, value):
        self._write('<h', value)

    def write_int32(self, value):
        self._write('<i', value)

    def write_int64(self, value):
        self._write('<q', value)

    def write_uint8(self, value):
        self._write('<B', value)

    def write_uint16(self, value):
        self._write('<H', value)

    def write_uint32(self, value):
        self._write('<I', value)

    def write_uint64(self, value):
        self._write('<Q', value)

    def write_float(self, value):
        self._write('<f', value)

    def write_double(self, value):
        self._write('<d', value)

    def write_string(self, value):
        self.write_binary(value.encode())

    def write_binary(self, value):
        self.write_uint32(len(value))
        self._buffer += value

    def start_write_list(self, length):
        self.write_uint32(length)

    def start_write_union(self, discriminator):
        self.write_uint32(discriminator)

    def write_null_flag(self, flag):
        self.write_uint8(1 if flag else 0)

    def start_read_message(self):
        """ Returns 4-tuple of msgType, service, request, sequence.

        Raises:
            CodecError: Raise this error when unsupported codec version doesn't match.

        Returns:
            codec.MessageInfo: 4-tuple of msgType, service, request, sequence.
        """
        header = self.read_uint32()
        sequence = self.read_uint32()
        version = header >> 24
        if version != self.BASIC_CODEC_VERSION:
            raise CodecError("unsupported codec version %d" % version)
        service = (header >> 16) & 0xff
        request = (header >> 8) & 0xff
        msgType = MessageType(header & 0xff)
        return MessageInfo(type=msgType, service=service, request=request, sequence=sequence)

    def _read(self, fmt):
        result = struct.unpack_from(fmt, self._buffer, self._cursor)
        self._cursor += struct.calcsize(fmt)
        return result[0]

    def read_bool(self):
        return self._read('<?')

    def read_int8(self):
        return self._read('<b')

    def read_int16(self):
        return self._read('<h')

    def read_int32(self):
        return self._read('<i')

    def read_int64(self):
        return self._read('<q')

    def read_uint8(self):
        return self._read('<B')

    def read_uint16(self):
        return self._read('<H')

    def read_uint32(self):
        return self._read('<I')

    def read_uint64(self):
        return self._read('<Q')

    def read_float(self):
        return self._read('<f')

    def read_double(self):
        return self._read('<d')

    def read_string(self):
        return self.read_binary().decode()

    def read_binary(self):
        length = self.read_uint32()
        data = self._buffer[self._cursor:self._cursor+length]
        self._cursor += length
        return data

    def start_read_list(self):
        """ Function which should be called on list de-serialization.

        Returns:
            int: Int of list length.
        """
        return self.read_uint32()

    def start_read_union(self):
        """ Function which should be called on union de-serialization.

        Returns:
            int: Int of union discriminator.
        """
        return self.read_int32()

    def read_null_flag(self):
        return self.read_uint8()
