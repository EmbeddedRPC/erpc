/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_port.h"

#include <rtthread.h>

void *erpc_malloc(size_t size)
{
    return rt_malloc(size);
}

void erpc_free(void *ptr)
{
    rt_free(ptr);
}
