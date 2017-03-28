/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
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

#ifndef _ERPC_ARBITRATED_CLIENT_SETUP_H_
#define _ERPC_ARBITRATED_CLIENT_SETUP_H_

#include <stdint.h>
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
erpc_transport_t erpc_arbitrated_client_init(erpc_transport_t transport);

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
