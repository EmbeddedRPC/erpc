/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_port.h"

#include <new>

extern "C" {
#include "FreeRTOS.h"
#include "MemManager.h"
}

#if !(__embedded_cplusplus)
using namespace std;
#endif

void *operator new(std::size_t count) THROW_BADALLOC
{
    void *p = erpc_malloc(count);
    return p;
}

void *operator new(std::size_t count, const std::nothrow_t &tag) THROW NOEXCEPT
{
    void *p = erpc_malloc(count);
    return p;
}

void *operator new[](std::size_t count) THROW_BADALLOC
{
    void *p = erpc_malloc(count);
    return p;
}

void *operator new[](std::size_t count, const std::nothrow_t &tag) THROW NOEXCEPT
{
    void *p = erpc_malloc(count);
    return p;
}

void operator delete(void *ptr)THROW
{
    erpc_free(ptr);
}

void operator delete[](void *ptr) THROW
{
    erpc_free(ptr);
}

void *erpc_malloc(size_t size)
{
    void *p = MEM_BufferAllocForever(size, 0);
    return p;
}

void erpc_free(void *ptr)
{
    MEM_BufferFree(ptr);
}

/* Provide function for pure virtual call to avoid huge demangling code being linked in ARM GCC */
#if ((defined(__GNUC__)) && (defined(__arm__)))
extern "C" void __cxa_pure_virtual(void)
{
    while (1)
        ;
}
#endif
