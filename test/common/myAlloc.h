/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__MYALLOC_H_
#define _EMBEDDED_RPC__MYALLOC_H_

#include "erpc_port.h"

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

class MyAlloc
{
public:
    static void *my_malloc(size_t allocation_size)
    {
        allocated_++;
        return erpc_malloc(allocation_size);
    }

    static void my_free(void *block)
    {
        allocated_--;
        erpc_free(block);
    }

    static int allocated() { return allocated_; }
    static void allocated(int allocated) { allocated_ = allocated; }

private:
    static int allocated_;
};

namespace std {
using ::MyAlloc;
}

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

#define erpc_malloc(X) MyAlloc::my_malloc(X)
#define erpc_free(X) MyAlloc::my_free((X))

#endif // _EMBEDDED_RPC__MYALLOC_H_
