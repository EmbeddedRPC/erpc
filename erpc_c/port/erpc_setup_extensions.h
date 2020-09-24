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

#if ERPC_THREADS

#if ERPC_THREADS_IS(FREERTOS)
#include "FreeRTOS.h"
#include "timers.h"
#include "task.h"
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
}

extern "C" {
#endif

/*!
 * @brief This function is used for creating default timer for task freeze detection.
 *
 * @param[in] waitTime Platform specific time to throw error cb in case of eRPC call freeze.
 */
void erpc_create_timer_default(uint32_t waitTime);

/*!
 * @brief This function returns default eRPC call progress status.
 *
 * @return True if eRPC call is in progress, otherwise False.
 */
bool is_erpc_call_executed_default(void);

/*!
 * @brief This function sets default eRPC call progress status.
 *
 * @param[in] erpc_call Set erpc call progress status.
 * (E.G. in case of frezee and user will reinitialize eRPC communication)
 */
void set_erpc_call_execution_state_default(bool erpc_call);

#ifdef __cplusplus
}
#endif

/*! @} */

#endif // _EMBEDDED_RPC__SETUP_EXTENSIONS_H_
