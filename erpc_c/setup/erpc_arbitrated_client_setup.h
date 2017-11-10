/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
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
 * o Neither the name of the copyright holder nor the names of its
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

#ifndef _ERPC_ARBITRATED_CLIENT_SETUP_H_
#define _ERPC_ARBITRATED_CLIENT_SETUP_H_

#include "erpc_common.h"
#include "erpc_config_internal.h"
#include "erpc_mbf_setup.h"
#if ERPC_NESTED_CALLS
#include "erpc_server_setup.h"
#endif
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

#include <stdint.h>

typedef void (*client_error_handler_t)(erpc_status_t err); /*!< eRPC error handler function type. */

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
 *      erpc_transport_t sharedSerial = erpc_transport_serial_init(...);
 *      erpc_transport_t arbitrator = erpc_arbitrated_client_init(sharedSerial);
 *      erpc_server_init(arbitrator);
 * @endcode
 *
 * @return Transport arbitrator reference that should be passed to the server setup API.
 */
erpc_transport_t erpc_arbitrated_client_init(erpc_transport_t transport, erpc_mbf_t message_buffer_factory);

/*!
 * @brief This function set error handler function.
 *
 * Given error_handler function is called when error occur inside eRPC infrastructure.
 *
 * @param[in] error_handler Pointer to function error handler.
 */
void erpc_client_set_error_handler(client_error_handler_t error_handler);

#if ERPC_NESTED_CALLS
/*!
 * @brief This function set server object for handling nested eRPC calls.
 *
 * @param[in] server Initiated server.
 */
void erpc_client_set_server(erpc_server_t server);
#endif

#if ERPC_MESSAGE_LOGGING
/*!
 * @brief This function adds transport object for logging send/receive messages.
 *
 * @param[in] transport Initiated transport.
 *
 * @retval True When transport was succesfully added.
 * @retval False When transport wasn't added.
 */
bool erpc_server_add_message_logger(erpc_transport_t transport);
#endif

/*!
 * @brief This function de-initializes client.
 *
 * This function de-initializes client and all components which it own.
 */
void erpc_arbitrated_client_deinit(void);

//@}

#ifdef __cplusplus
}
#endif

/*! @} */

#endif // _ERPC_ARBITRATED_CLIENT_SETUP_H_
