/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__ERROR_HANDLER_H_
#define _EMBEDDED_RPC__ERROR_HANDLER_H_

#include "erpc_common.h"
#include <stdint.h>

/*!
 * @addtogroup error_handler
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// API
////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

//! @name Error handler
//@{

/*!
 * @brief This function handles eRPC errors.
 *
 * This function prints a description of occurred error and sets bool variable g_erpc_error_occurred which is used for
 * determining if error occurred in user application on client side.
 */
void erpc_error_handler(erpc_status_t err, uint32_t functionID);

//@}

#ifdef __cplusplus
}
#endif

/*! @} */
#endif // _EMBEDDED_RPC__ERROR_HANDLER_H_
