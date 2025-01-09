#!/usr/bin/env python

# Copyright (c) 2015-2016 Freescale Semiconductor, Inc.
# Copyright 2016-2025 NXP
#
# SPDX-License-Identifier: BSD-3-Clause

from enum import Enum
from typing import NamedTuple


class MessageType(Enum):
    kInvocationMessage = 0
    kOnewayMessage = 1
    kReplyMessage = 2
    kNotificationMessage = 3


MessageInfo = NamedTuple(
    'MessageInfo', [('type', MessageType), ('service', int), ('request', int), ('sequence', int)])


class CodecError(RuntimeError):
    pass


class Codec:
    def __init__(self):
        self._buffer = bytearray()
        self._cursor = 0

    @property
    def buffer(self) -> bytearray:
        return self._buffer

    @buffer.setter
    def buffer(self, buf: bytearray):
        self._buffer = buf
        self._cursor = 0

    def reset(self):
        self._buffer = bytearray()
        self._cursor = 0

    def start_write_message(self, msgInfo: MessageInfo) -> None:
        raise NotImplementedError()

    def write_bool(self, value: bool) -> None:
        raise NotImplementedError()

    def write_int8(self, value: int) -> None:
        raise NotImplementedError()

    def write_int16(self, value: int) -> None:
        raise NotImplementedError()

    def write_int32(self, value: int) -> None:
        raise NotImplementedError()

    def write_int64(self, value: int) -> None:
        raise NotImplementedError()

    def write_uint8(self, value: int) -> None:
        raise NotImplementedError()

    def write_uint16(self, value: int) -> None:
        raise NotImplementedError()

    def write_uint32(self, value: int) -> None:
        raise NotImplementedError()

    def write_uint64(self, value: int) -> None:
        raise NotImplementedError()

    def write_float(self, value: float) -> None:
        raise NotImplementedError()

    def write_double(self, value: float) -> None:
        raise NotImplementedError()

    def write_string(self, value: str) -> None:
        raise NotImplementedError()

    def write_binary(self, value: bytes) -> None:
        raise NotImplementedError()

    def start_write_list(self, length: int) -> None:
        raise NotImplementedError()

    def start_write_union(self, discriminator: int) -> None:
        raise NotImplementedError()

    def write_null_flag(self, flag: int) -> None:
        raise NotImplementedError()

    def start_read_message(self) -> MessageInfo:
        """ Returns MessageInfo object.

        Returns:
            MessageInfo: MessageInfo object.
        """
        raise NotImplementedError()

    def read_bool(self) -> bool:
        raise NotImplementedError()

    def read_int8(self) -> int:
        raise NotImplementedError()

    def read_int16(self) -> int:
        raise NotImplementedError()

    def read_int32(self) -> int:
        raise NotImplementedError()

    def read_int64(self) -> int:
        raise NotImplementedError()

    def read_uint8(self) -> int:
        raise NotImplementedError()

    def read_uint16(self) -> int:
        raise NotImplementedError()

    def read_uint32(self) -> int:
        raise NotImplementedError()

    def read_uint64(self) -> int:
        raise NotImplementedError()

    def read_float(self) -> float:
        raise NotImplementedError()

    def read_double(self) -> float:
        raise NotImplementedError()

    def read_string(self) -> str:
        raise NotImplementedError()

    def read_binary(self) -> bytearray:
        raise NotImplementedError()

    def start_read_list(self) -> int:
        """ Function which should be called on list de-serialization.

        Returns:
            int: Int of list length.
        """
        raise NotImplementedError()

    def start_read_union(self) -> int:
        """ Function which should be called on union de-serialization.

        Returns:
            int: Int of union discriminator.
        """
        raise NotImplementedError()

    def read_null_flag(self) -> int:
        raise NotImplementedError()
