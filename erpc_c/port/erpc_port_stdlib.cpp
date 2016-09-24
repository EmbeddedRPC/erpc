/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "erpc_port.h"
#include <new>
#include <stdlib.h>

#if __cplusplus >= 201103
#define NOEXCEPT noexcept
#else
#define NOEXCEPT
#endif // NOEXCEPT

#if !(__embedded_cplusplus)
using namespace std;
#endif

#if defined(__CC_ARM) /* Keil MDK */
void *operator new(std::size_t count) throw(std::bad_alloc)
{
    void *p = erpc_malloc(count);
    return p;
}

void *operator new(std::size_t count, const std::nothrow_t &tag) throw() NOEXCEPT
{
    void *p = erpc_malloc(count);
    return p;
}

void *operator new[](std::size_t count) throw(std::bad_alloc)
{
    void *p = erpc_malloc(count);
    return p;
}

void *operator new[](std::size_t count, const std::nothrow_t &tag) throw() NOEXCEPT

{
    void *p = erpc_malloc(count);
    return p;
}

void operator delete(void *ptr) throw() NOEXCEPT
{
    erpc_free(ptr);
}

void operator delete[](void *ptr) throw() NOEXCEPT
{
    erpc_free(ptr);
}

#else

void *operator new(size_t count)
{
    void *p = erpc_malloc(count);
    return p;
}

void *operator new(size_t count, const nothrow_t &tag) NOEXCEPT
{
    void *p = erpc_malloc(count);
    return p;
}

void *operator new[](size_t count)
{
    void *p = erpc_malloc(count);
    return p;
}

void *operator new[](size_t count, const nothrow_t &tag) NOEXCEPT
{
    void *p = erpc_malloc(count);
    return p;
}

void operator delete(void *ptr)NOEXCEPT
{
    erpc_free(ptr);
}

void operator delete[](void *ptr) NOEXCEPT
{
    erpc_free(ptr);
}
#endif

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
extern "C" void __cxa_pure_virtual()
{
    while (1)
        ;
}
#endif
