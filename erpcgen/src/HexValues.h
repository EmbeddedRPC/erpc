/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#if !defined(_HexValues_h_)
#define _HexValues_h_

#include <cstdbool>
#include <cstdint>

//! \brief Determines whether \a c is a hex digit character.
bool isHexDigit(char c);

//! \brief Converts a hexadecimal character to the integer equivalent.
uint8_t hexCharToInt(char c);

//! \brief Converts a hex-encoded byte to the integer equivalent.
uint8_t hexByteToInt(const char *encodedByte);

#endif // _HexValues_h_
