/*
 * Copyright 2021-2025 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_crc16.hpp"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

Crc16::Crc16(uint32_t crcStart) : m_crcStart(crcStart) {}

Crc16::Crc16(void) : m_crcStart(0xEF4A) {}

Crc16::~Crc16(void) {}

uint16_t Crc16::computeCRC16(const uint8_t *data, uint32_t lengthInBytes)
{
    uint32_t crc = m_crcStart;
    uint32_t j;
    uint32_t i;
    uint32_t byte;
    uint32_t temp;
    const uint32_t andValue = 0x8000U;
    const uint32_t xorValue = 0x1021U;

    for (j = 0; j < lengthInBytes; ++j)
    {
        byte = data[j];
        crc ^= byte << 8;
        for (i = 0; i < 8U; ++i)
        {
            temp = crc << 1;
            if (0UL != (crc & andValue))
            {
                temp ^= xorValue;
            }
            crc = temp;
        }
    }

    return (uint16_t)(crc & 0xFFFFU);
}
void Crc16::setCrcStart(uint32_t crcStart)
{
    m_crcStart = crcStart;
}
