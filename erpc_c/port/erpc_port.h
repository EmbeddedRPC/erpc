/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__PORT_H_
#define _EMBEDDED_RPC__PORT_H_

#include "erpc_config_internal.h"

/*!
 * @addtogroup port_mem
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Declarations
////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/*!
 * @brief This function is used for allocating space.
 *
 * @param[in] size Size of memory block which should be allocated.
 *
 * @return Pointer to allocated space or NULL.
 */
void *erpc_malloc(size_t size);

/*!
 * @brief This function free given memory block.
 *
 * @param[in] ptr Pointer to memory which should be freed.
 */
void erpc_free(void *ptr);

#ifdef __cplusplus
};
#endif

/*! @} */

#endif // _EMBEDDED_RPC__PORT_H_
