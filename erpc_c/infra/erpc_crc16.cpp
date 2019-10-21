/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_crc16.h"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

Crc16::Crc16(uint32_t crcStart)
: m_crcStart(crcStart)
{
}

Crc16::Crc16(void)
: m_crcStart(0xEF4A)
{
}

Crc16::~Crc16(void) {}

uint16_t Crc16::computeCRC16(const uint8_t *data, uint32_t lengthInBytes)
{
    uint32_t crc = m_crcStart;

    uint32_t j;
    for (j = 0; j < lengthInBytes; ++j)
    {
        uint32_t i;
        uint32_t byte = data[j];
        crc ^= byte << 8;
        for (i = 0; i < 8; ++i)
        {
            uint32_t temp = crc << 1;
            if (crc & 0x8000)
            {
                temp ^= 0x1021;
            }
            crc = temp;
        }
    }

    return crc;
}
void Crc16::setCrcStart(uint32_t crcStart)
{
    m_crcStart = crcStart;
}
