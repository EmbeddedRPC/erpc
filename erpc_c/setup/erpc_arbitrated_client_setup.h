/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * Copyright 2019 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _ERPC_ARBITRATED_CLIENT_SETUP_H_
#define _ERPC_ARBITRATED_CLIENT_SETUP_H_

#include "erpc_common.h"
#include "erpc_config_internal.h"
#include "erpc_mbf_setup.h"
#if ERPC_PRE_POST_ACTION
#include "erpc_pre_post_action.h"
#endif
#if ERPC_NESTED_CALLS
#include "erpc_server_setup.h"
#endif
#include "erpc_client_manager.h"
#include "erpc_transport_setup.h"

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

#include <stdbool.h>
#include <stdint.h>

//! @name Arbitrated client setup
//@{

/*!
 * @brief Initializes a client that shares its transport with the server.
 *
 * This function initializes a client with all components necessary for sending client requests
 * using the same transport instance as used by a server. Only one instance of the shared
 * transport should be created. The transport arbitrator that wraps the shared transport is
 * returned. This arbitrator, not the shared transport, should be passed to the server setup
 * routine.
 *
 * Example use:
 * @code
 *      erpc_transport_t arbitrator;
 *      erpc_transport_t sharedSerial = erpc_transport_serial_init(...);
 *      erpc_client_t client = erpc_arbitrated_client_init(sharedSerial, message_buffer_factory, &arbitrator);
 *      erpc_server_init(arbitrator);
 * @endcode
 *
 * @param[in] transport Initiated transport.
 * @param[in] message_buffer_factory Initiated message buffer factory.
 * @param[out] arbitrator Transport arbitrator reference that should be passed to the server setup API.
 *
 * @return erpc_client_t Pointer to client structure.
 */
erpc_client_t erpc_arbitrated_client_init(erpc_transport_t transport, erpc_mbf_t message_buffer_factory,
                                          erpc_transport_t *arbitrator);

/*!
 * @brief This function sets error handler function.
 *
 * Given error_handler function is called when error occur inside eRPC infrastructure.
 *
 * @param[in] client Pointer to client structure.
 * @param[in] error_handler Pointer to function error handler.
 */
void erpc_arbitrated_client_set_error_handler(erpc_client_t client, client_error_handler_t error_handler);

/*!
 * @brief Can be used to set own crcStart number.
 *
 * For example can be used generated crc from erpcgen
 * which is providing when @crc annotation is used.
 * Accessed can be through 'extern const uint32_t erpc_generated_crc;'
 *
 * @param[in] client Pointer to client structure.
 * @param[in] crcStart Set start number for crc.
 */
void erpc_arbitrated_client_set_crc(erpc_client_t client, uint32_t crcStart);

#if ERPC_NESTED_CALLS
/*!
 * @brief This function sets server object for handling nested eRPC calls.
 *
 * @param[in] client Pointer to client structure.
 * @param[in] server Initiated server.
 */
void erpc_arbitrated_client_set_server(erpc_client_t client, erpc_server_t server);

/*!
 * @brief This function sets server thread id.
 *
 * @param[in] client Pointer to client structure.
 * @param[in] serverThreadId Id of thread where server run function is executed.
 */
void erpc_arbitrated_client_set_server_thread_id(erpc_client_t client, void *serverThreadId);
#endif

#if ERPC_MESSAGE_LOGGING
/*!
 * @brief This function adds transport object for logging send/receive messages.
 *
 * @param[in] client Pointer to client structure.
 * @param[in] transport Initiated transport.
 *
 * @retval True When transport was successfully added.
 * @retval False When transport wasn't added.
 */
bool erpc_arbitrated_client_add_message_logger(erpc_client_t client, erpc_transport_t transport);
#endif

#if ERPC_PRE_POST_ACTION
/*!
 * @brief This function set callback function executed at the beginning of eRPC call.
 *
 * @param[in] client Pointer to client structure.
 * @param[in] preCB Callback used at the beginning of eRPC call. When NULL and ERPC_PRE_POST_ACTION_DEFAULT
 * is enabled then default function will be set.
 */
void erpc_arbitrated_client_add_pre_cb_action(erpc_client_t client, pre_post_action_cb preCB);

/*!
 * @brief This function set callback function executed at the end of eRPC call.
 *
 * @param[in] client Pointer to client structure.
 * @param[in] postCB Callback used at the end of eRPC call. When NULL and ERPC_PRE_POST_ACTION_DEFAULT
 * is enabled then default function will be set.
 */
void erpc_arbitrated_client_add_post_cb_action(erpc_client_t client, pre_post_action_cb postCB);
#endif

/*!
 * @brief This function de-initializes client.
 *
 * This function de-initializes client and all components which it own.
 *
 * @param[in] client Pointer to client structure.
 */
void erpc_arbitrated_client_deinit(erpc_client_t client);

//@}

#ifdef __cplusplus
}
#endif

/*! @} */

#endif // _ERPC_ARBITRATED_CLIENT_SETUP_H_
