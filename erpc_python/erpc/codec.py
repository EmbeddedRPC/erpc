#!/usr/bin/env python

# Copyright (c) 2015-2016 Freescale Semiconductor, Inc.
# Copyright 2016-2017 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

from enum import Enum
from collections import namedtuple


class MessageType(Enum):
    kInvocationMessage = 0
    kOnewayMessage = 1
    kReplyMessage = 2
    kNotificationMessage = 3


MessageInfo = namedtuple(
    'MessageInfo', ['type', 'service', 'request', 'sequence'])


class CodecError(RuntimeError):
    pass


class Codec(object):
    def __init__(self):
        self.reset()

    @property
    def buffer(self):
        return self._buffer

    @buffer.setter
    def buffer(self, buf):
        self._buffer = buf
        self._cursor = 0

    def reset(self):
        self._buffer = bytearray()
        self._cursor = 0

    def start_write_message(self, msgInfo):
        raise NotImplementedError()

    def write_bool(self, value):
        raise NotImplementedError()

    def write_int8(self, value):
        raise NotImplementedError()

    def write_int16(self, value):
        raise NotImplementedError()

    def write_int32(self, value):
        raise NotImplementedError()

    def write_int64(self, value):
        raise NotImplementedError()

    def write_uint8(self, value):
        raise NotImplementedError()

    def write_uint16(self, value):
        raise NotImplementedError()

    def write_uint32(self, value):
        raise NotImplementedError()

    def write_uint64(self, value):
        raise NotImplementedError()

    def write_float(self, value):
        raise NotImplementedError()

    def write_double(self, value):
        raise NotImplementedError()

    def write_string(self, value):
        raise NotImplementedError()

    def write_binary(self, value):
        raise NotImplementedError()

    def start_write_list(self, length):
        raise NotImplementedError()

    def start_write_union(self, discriminator):
        raise NotImplementedError()

    def write_null_flag(self, flag):
        raise NotImplementedError()

    def start_read_message(self):
        """ Returns MessageInfo object.

        Returns:
            MessageInfo: MessageInfo object.
        """
        raise NotImplementedError()

    def read_bool(self):
        raise NotImplementedError()

    def read_int8(self):
        raise NotImplementedError()

    def read_int16(self):
        raise NotImplementedError()

    def read_int32(self):
        raise NotImplementedError()

    def read_int64(self):
        raise NotImplementedError()

    def read_uint8(self):
        raise NotImplementedError()

    def read_uint16(self):
        raise NotImplementedError()

    def read_uint32(self):
        raise NotImplementedError()

    def read_uint64(self):
        raise NotImplementedError()

    def read_float(self):
        raise NotImplementedError()

    def read_double(self):
        raise NotImplementedError()

    def read_string(self):
        raise NotImplementedError()

    def read_binary(self):
        raise NotImplementedError()

    def start_read_list(self):
        """ Function which should be called on list de-serialization.

        Returns:
            int: Int of list length.
        """
        raise NotImplementedError()

    def start_read_union(self):
        """ Function which should be called on union de-serialization.

        Returns:
            int: Int of union discriminator.
        """
        raise NotImplementedError()

    def read_null_flag(self):
        raise NotImplementedError()
