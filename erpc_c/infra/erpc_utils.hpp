/*
 * Copyright 2023 ACRIOS Systems s.r.o.
 * Copyright 2026 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__UTILS_H_
#define _EMBEDDED_RPC__UTILS_H_

#include <stdint.h>

namespace erpc {
typedef void *functionPtr_t;
typedef functionPtr_t *arrayOfFunctionPtr_t;

bool findIndexOfFunction(const arrayOfFunctionPtr_t sourceArrayOfFunctionPtr, uint16_t sourceArrayLength,
                         const functionPtr_t functionPtr, uint16_t &retVal);
} // namespace erpc
#endif // _EMBEDDED_RPC__UTILS_H_
