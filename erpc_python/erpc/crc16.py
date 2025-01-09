#!/usr/bin/env python

# Copyright (c) 2016 Freescale Semiconductor, Inc.
# Copyright 2016-2025 NXP
#
# SPDX-License-Identifier: BSD-3-Clause

from typing import Union

class Crc16:
    POLY = 0x1021

    def __init__(self, crcStart: int=0xEF4A):
        self._crc_start = crcStart
        self._table = [self.compute_table(i) for i in range(256)]

    def compute_table(self, i: int):
        crc = 0
        i <<= 8
        for _ in range(8):
            temp = crc ^ i
            crc <<= 1
            if temp & 0x8000:
                crc ^= self.POLY
            i <<= 1
        return crc

    def compute_crc16(self, data: Union[str, bytes]) -> int:
        if isinstance(data, str):
            data = bytes(data.encode())
        crc = self._crc_start
        for c in data:
            crc = ((crc << 8) ^ self._table[((crc >> 8) ^ c) & 0xff]) & 0xffff
        return crc
