/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__CLIENT_SETUP_H_
#define _EMBEDDED_RPC__CLIENT_SETUP_H_

#include "erpc_common.h"
#include "erpc_config_internal.h"
#include "erpc_mbf_setup.h"
#if ERPC_NESTED_CALLS
#include "erpc_server_setup.h"
#endif
#include "erpc_client_manager.h"
#include "erpc_transport_setup.h"
#include <stdbool.h>

/*!
 * @addtogroup client_setup
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// API
////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

//! @name Client setup
//@{

/*!
 * @brief This function initializes client.
 *
 * @param[in] transport Initiated transport.
 * @param[in] message_buffer_factory Initiated message buffer factory.
 *
 * This function initializes client with all components necessary for serve client request.
 */
void erpc_client_init(erpc_transport_t transport, erpc_mbf_t message_buffer_factory);

/*!
 * @brief This function sets error handler function.
 *
 * Given error_handler function is called when error occur inside eRPC infrastructure.
 *
 * @param[in] error_handler Pointer to function error handler.
 */
void erpc_client_set_error_handler(client_error_handler_t error_handler);

/*!
 * @brief Can be used to set own crcStart number.
 *
 * For example can be used generated crc from erpcgen
 * which is providing when @crc annotation is used.
 * Accessed can be through 'extern const uint32_t erpc_generated_crc;'
 *
 * @param[in] crcStart Set start number for crc.
 */
void erpc_client_set_crc(uint32_t crcStart);

#if ERPC_NESTED_CALLS
/*!
 * @brief This function sets server object for handling nested eRPC calls.
 *
 * @param[in] server Initiated server.
 */
void erpc_client_set_server(erpc_server_t server);

/*!
 * @brief This function sets server thread id.
 *
 * @param[in] serverThreadId Id of thread where server run function is executed.
 */
void erpc_client_set_server_thread_id(void *serverThreadId);
#endif

#if ERPC_MESSAGE_LOGGING
/*!
 * @brief This function adds transport object for logging send/receive messages.
 *
 * @param[in] transport Initiated transport.
 *
 * @retval True When transport was successfully added.
 * @retval False When transport wasn't added.
 */
bool erpc_client_add_message_logger(erpc_transport_t transport);
#endif

/*!
 * @brief This function de-initializes client.
 *
 * This function de-initializes client and all components which it own.
 */
void erpc_client_deinit(void);

//@}

#ifdef __cplusplus
}
#endif

/*! @} */

#endif // _EMBEDDED_RPC__CLIENT_SETUP_H_
