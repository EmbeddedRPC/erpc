/*
 * Copyright 2020 NXP
 * Copyright 2020 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__SETUP_EXTENSIONS_H_
#define _EMBEDDED_RPC__SETUP_EXTENSIONS_H_

#include "erpc_config_internal.h"

#include <stdint.h>

#if !ERPC_THREADS_IS(NONE)

#if ERPC_THREADS_IS(FREERTOS)
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#endif

#endif // ERPC_THREADS

/*!
 * @addtogroup port_setup_extensions
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Declarations
////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus

namespace erpc {
/*!
 * @brief This function is used for default pre erpc call action.
 */
void erpc_pre_cb_default(void);

/*!
 * @brief This function is used for default post erpc call action.
 */
void erpc_post_cb_default(void);
} // namespace erpc

extern "C" {
#endif

#if ERPC_THREADS_IS(FREERTOS)
typedef TimerCallbackFunction_t erpc_call_timer_cb_default_t;
#else
typedef void *erpc_call_timer_cb_default_t;
#endif

/*!
 * @brief This function is used for initializing variables for task freeze detection.
 *
 * @param[in] erpc_call_timer_cb Callback function called when eRPC call freeze.
 * When NULL default callback will be used.
 * @param[in] waitTimeMs Platform specific time to throw error cb in case of eRPC call freeze in [ms].
 */
void erpc_init_call_progress_detection_default(erpc_call_timer_cb_default_t erpc_call_timer_cb, uint32_t waitTimeMs);

/*!
 * @brief This function is used for deinitialization variables for task freeze detection.
 */
void erpc_deinit_call_progress_detection_default(void);

/*!
 * @brief This function returns default eRPC call progress status.
 *
 * @return True if eRPC call is in progress, otherwise False.
 */
bool erpc_is_call_in_progress_default(void);

/*!
 * @brief This function resets eRPC state.
 *
 * Can be used when user reinitialize communication between client and server.
 */
void erpc_reset_in_progress_state_default(void);

#ifdef __cplusplus
}
#endif

/*! @} */

#endif // _EMBEDDED_RPC__SETUP_EXTENSIONS_H_
