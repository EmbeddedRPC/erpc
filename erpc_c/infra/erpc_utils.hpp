/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

namespace erpc {
typedef void *functionPtr_t;
typedef functionPtr_t *arrayOfFunctionPtr_t;

bool findIndexOfFunction(const arrayOfFunctionPtr_t sourceArrayOfFunctionPtr, uint16_t sourceArrayLength,
                             const functionPtr_t functionPtr, uint16_t &retVal);
}
