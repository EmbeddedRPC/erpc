/*
 * Copyright (c) 2020, Embedded Planet, Inc
 * All rights reserved.
 *
 * For supporting transports and examples see:
 * https://github.com/EmbeddedPlanet/mbed-rpc
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_port.h"

#if ERPC_THREADS_IS(MBED)

#include <cstdlib>

using namespace std;

void *erpc_malloc(size_t size)
{
    void *p = malloc(size);
    return p;
}

void erpc_free(void *ptr)
{
    free(ptr);
}
#endif
