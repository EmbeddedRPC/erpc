/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
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
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
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

#ifndef _EMBEDDED_RPC__SERVER_SETUP_H_
#define _EMBEDDED_RPC__SERVER_SETUP_H_

#include "erpc_transport_setup.h"
#include "erpc_common.h"
#include <stdint.h>
#include <stdbool.h>

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

//! @name Server setup
//@{

/*!
 * @brief This function initializes server.
 *
 * This function initializes server with all components necessary for running server.
 */
void erpc_server_init(erpc_transport_t transport);

/*!
 * @brief This function de-initializes server.
 *
 * This function de-initializes server and all components which it own.
 */
void erpc_server_deinit(void);

/*!
 * @brief This function adds service to server.
 *
 * Services contain implementations of functions called from client to server.
 *
 * @param[in] service Service which contains implementations of functions called from client to server.
 */
void erpc_add_service_to_server(void *service);

//@}

//! @name Server control
//@{

/*!
 * @brief This function calls server implementation until it is stopped.
 *
 * This is blocking method, where server is trying read (and if it is requested also send) message
 * until it is stopped.
 *
 * @return Return one of status from erpc_common.h
 */
erpc_status_t erpc_server_run(void);

/*!
 * @brief This function calls server implementation only once.
 *
 * This is non-blocking method, where server is trying read (and if it is requested also send) message only once.
 *
 * @return Return one of status from erpc_common.h
 */
erpc_status_t erpc_server_poll(void);

/*!
 * @brief This functions should be used when client is calling quit server.
 *
 * This method sets server from On to OFF. When the server returns from its implementation,
 * erpc_server_deinit() function should be called.
 */
void erpc_server_stop(void);

//@}

#ifdef __cplusplus
}
#endif

/*! @} */

#endif // _EMBEDDED_RPC__SERVER_SETUP_H_
