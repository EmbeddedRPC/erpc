/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_port.h"

#include <cstdlib>
#include <new>

using namespace std;

void *operator new(size_t count) THROW_BADALLOC
{
    void *p = erpc_malloc(count);
    return p;
}

void *operator new(size_t count, const nothrow_t &tag) THROW NOEXCEPT
{
    (void)tag;
    void *p = erpc_malloc(count);
    return p;
}

void *operator new[](size_t count) THROW_BADALLOC
{
    void *p = erpc_malloc(count);
    return p;
}

void *operator new[](size_t count, const nothrow_t &tag) THROW NOEXCEPT
{
    (void)tag;
    void *p = erpc_malloc(count);
    return p;
}

void operator delete(void *ptr)THROW NOEXCEPT
{
    erpc_free(ptr);
}

void operator delete(void *ptr, std::size_t count)THROW NOEXCEPT
{
    (void)count;
    erpc_free(ptr);
}

void operator delete[](void *ptr) THROW NOEXCEPT
{
    erpc_free(ptr);
}

void operator delete[](void *ptr, std::size_t count) THROW NOEXCEPT
{
    (void)count;
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
    while (1)
        ;
}
#endif
