/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__EXTERNAL_H_
#define _EMBEDDED_RPC__EXTERNAL_H_

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

#include <stdint.h>

// Enumerators data types declarations
typedef enum myEnum
{
    one = 0,
    two = 1,
    three = 2
} myEnum;

// Aliases data types declarations
typedef int32_t myInt;

typedef struct fooStruct fooStruct;

// Structures data types declarations
struct fooStruct
{
    int32_t x;
    float y;
};

// Constant variable declarations
const int32_t i = 4;

#endif // _EMBEDDED_RPC__EXTERNAL_H_
