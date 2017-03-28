#!/usr/bin/env python

# Copyright (c) 2015-2016 Freescale Semiconductor, Inc.
# Copyright 2016-2017 NXP
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
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

from enum import Enum
from collections import namedtuple

class MessageType(Enum):
    kInvocationMessage = 0
    kOnewayMessage = 1
    kReplyMessage = 2
    kNotificationMessage = 3

MessageInfo = namedtuple('MessageInfo', ['type', 'service', 'request', 'sequence'])

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

    def end_write_message(self):
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

    def end_write_list(self):
        raise NotImplementedError()

    def start_write_struct(self):
        raise NotImplementedError()

    def end_write_struct(self):
        raise NotImplementedError()

    def start_write_union(self, discriminator):
        raise NotImplementedError()

    def end_write_union(self):
        raise NotImplementedError()

    def write_null_flag(self, flag):
        raise NotImplementedError()

    ##
    # @return MessageInfo object.
    def start_read_message(self):
        raise NotImplementedError()

    def end_read_message(self):
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

    ##
    # @return Int of list length.
    def start_read_list(self):
        raise NotImplementedError()

    def end_read_list(self):
        raise NotImplementedError()

    def start_read_struct(self):
        raise NotImplementedError()

    def end_read_struct(self):
        raise NotImplementedError()

    ##
    # @return Int of union discriminator.
    def start_read_list(self):
        raise NotImplementedError()

    def end_read_list(self):
        raise NotImplementedError()

    def read_null_flag(self):
        raise NotImplementedError()




