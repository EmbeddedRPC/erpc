/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * Copyright 2020 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__SERVER_SETUP_H_
#define _EMBEDDED_RPC__SERVER_SETUP_H_

#include "erpc_common.h"
#include "erpc_config_internal.h"
#include "erpc_mbf_setup.h"
#include "erpc_transport_setup.h"
#if ERPC_PRE_POST_ACTION
#include "erpc_pre_post_action.h"
#endif

/*!
 * @addtogroup server_setup
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

//! @brief Opaque server object type.
typedef struct ServerType *erpc_server_t;

//! @name Server setup
//@{

/*!
 * @brief This function initializes server.
 *
 * This function initializes server with all components necessary for running server.
 *
 * @param[in] transport Initiated transport.
 * @param[in] message_buffer_factory Initiated message buffer factory.
 *
 * @return erpc_server_t Pointer to server structure.
 */
erpc_server_t erpc_server_init(erpc_transport_t transport, erpc_mbf_t message_buffer_factory);

/*!
 * @brief This function de-initializes server.
 *
 * This function de-initializes server and all components which it own.
 *
 * @param[in] server Pointer to server structure.
 */
void erpc_server_deinit(erpc_server_t server);

/*!
 * @brief This function adds service to server.
 *
 * Services contain implementations of functions called from client to server.
 *
 * @param[in] server Pointer to server structure.
 * @param[in] service Service which contains implementations of functions called from client to server.
 */
void erpc_add_service_to_server(erpc_server_t server, void *service);

/*!
 * @brief This function removes service from server.
 *
 * @param[in] server Pointer to server structure.
 * @param[in] service Service which contains implementations of functions called from client to server.
 */
void erpc_remove_service_from_server(erpc_server_t server, void *service);

/*!
 * @brief Can be used to set own crcStart number.
 *
 * For example can be used generated crc from erpcgen
 * which is providing when @crc annotation is used.
 * Accessed can be through 'extern const uint32_t erpc_generated_crc;'
 *
 * @param[in] server Pointer to server structure.
 * @param[in] crcStart Set start number for crc.
 */
void erpc_server_set_crc(erpc_server_t server, uint32_t crcStart);
//@}

//! @name Server control
//@{

/*!
 * @brief This function calls server implementation until it is stopped.
 *
 * This is blocking method, where server is trying read (and if it is requested also send) message
 * until it is stopped.
 *
 * @param[in] server Pointer to server structure.
 *
 * @return Return one of status from erpc_common.h
 */
erpc_status_t erpc_server_run(erpc_server_t server);

/*!
 * @brief This function calls server implementation only once.
 *
 * This is non-blocking method, where server is trying read (and if it is requested also send) message only once.
 *
 * @param[in] server Pointer to server structure.
 *
 * @return Return one of status from erpc_common.h
 */
erpc_status_t erpc_server_poll(erpc_server_t server);

/*!
 * @brief This functions should be used when client is calling quit server.
 *
 * This method sets server from On to OFF. When the server returns from its implementation,
 * erpc_server_deinit() function should be called.
 *
 * @param[in] server Pointer to server structure.
 */
void erpc_server_stop(erpc_server_t server);

#if ERPC_MESSAGE_LOGGING
/*!
 * @brief This function adds transport object for logging send/receive messages.
 *
 * @param[in] server Pointer to server structure.
 * @param[in] transport Initiated transport.
 *
 * @retval True When transport was successfully added.
 * @retval False When transport wasn't added.
 */
bool erpc_server_add_message_logger(erpc_server_t server, erpc_transport_t transport);
#endif

#if ERPC_PRE_POST_ACTION
/*!
 * @brief This function set callback function executed at the beginning of eRPC call.
 *
 * @param[in] server Pointer to server structure.
 * @param[in] preCB Callback used at the beginning of eRPC call. When NULL and ERPC_PRE_POST_ACTION_DEFAULT
 * is enabled then default function will be set.
 */
void erpc_server_add_pre_cb_action(erpc_server_t server, pre_post_action_cb preCB);

/*!
 * @brief This function set callback function executed at the end of eRPC call.
 *
 * @param[in] server Pointer to server structure.
 * @param[in] postCB Callback used at the end of eRPC call. When NULL and ERPC_PRE_POST_ACTION_DEFAULT
 * is enabled then default function will be set.
 */
void erpc_server_add_post_cb_action(erpc_server_t server, pre_post_action_cb postCB);
#endif

//@}

#ifdef __cplusplus
}
#endif

/*! @} */

#endif // _EMBEDDED_RPC__SERVER_SETUP_H_
