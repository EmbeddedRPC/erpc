#!/usr/bin/env python

# Copyright (c) 2016 Freescale Semiconductor, Inc.
# Copyright 2016-2017 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

class Crc16(object):
    POLY = 0x1021

    def __init__(self, crcStart = 0xEF4A):
        self._crcStart = crcStart
        self._table = [self.compute_table(i) for i in range(256)]

    def compute_table(self, i):
        crc = 0
        i <<= 8
        for b in range(8):
            temp = crc ^ i
            crc <<= 1
            if temp & 0x8000:
                crc ^= self.POLY
            i <<= 1
        return crc

    def computeCRC16(self, data):
        if isinstance(data, str):
            data = bytearray(data)
        crc = self._crcStart
        for c in data:
            crc = ((crc << 8) ^ self._table[((crc >> 8) ^ c) & 0xff]) & 0xffff
        return crc
