/*
/*
 * Copyright (c) 2021, StarGate, Inc.
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * Ibrahim ERTURK <ierturk@ieee.org>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_port.h"

#include <new>

extern "C" {
#include "tx_api.h"
};

using namespace std;
void *operator new(std::size_t count) THROW_BADALLOC
{
    void *p = erpc_malloc(count);
    return p;
}

void *operator new(std::size_t count, const std::nothrow_t &tag) THROW
{
    (void)tag;
    void *p = erpc_malloc(count);
    return p;
}

void *operator new[](std::size_t count) THROW_BADALLOC
{
    void *p = erpc_malloc(count);
    return p;
}

void *operator new[](std::size_t count, const std::nothrow_t &tag) THROW
{
    (void)tag;
    void *p = erpc_malloc(count);
    return p;
}

void operator delete(void *ptr) THROW
{
    erpc_free(ptr);
}

void operator delete[](void *ptr) THROW
{
    erpc_free(ptr);
}

void *erpc_malloc(size_t size)
{
    void *p = malloc(size);
    return p;
}

void erpc_free(void *ptr)
{
    free(ptr);
}

/* Provide function for pure virtual call to avoid huge demangling code being linked in ARM GCC */
#if ((defined(__GNUC__)) && (defined(__arm__)))
extern "C" void __cxa_pure_virtual(void)
{
    for (;;)
    {
    };
}
#endif
