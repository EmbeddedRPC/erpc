/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "HexValues.h"
#include <cctype>

bool isHexDigit(char c)
{
    return isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

//! \return The integer equivalent to \a c.
//! \retval -1 The character \a c is not a hex character.
uint8_t hexCharToInt(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    else if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    else if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    else
        return static_cast<uint8_t>(-1);
}

//! \param encodedByte Must point to at least two ASCII hex characters.
//!
uint8_t hexByteToInt(const char *encodedByte)
{
    return (hexCharToInt(encodedByte[0]) << 4) | hexCharToInt(encodedByte[1]);
}
