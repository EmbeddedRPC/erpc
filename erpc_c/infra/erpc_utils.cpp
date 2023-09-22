/*
 * Copyright 2023 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_utils.hpp"

bool erpc::findIndexOfFunction(const arrayOfFunctionPtr_t sourceArrayOfFunctionPtr, uint16_t sourceArrayLength,
                               const functionPtr_t functionPtr, uint16_t &retVal)
{
    uint32_t index;
    bool find = false;
    for (index = 0; index < sourceArrayLength; index++)
    {
        if (sourceArrayOfFunctionPtr[index] == functionPtr)
        {
            retVal = index;
            find = true;
            break;
        }
    }
    return find;
}
