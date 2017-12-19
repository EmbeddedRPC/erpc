/*
 * The Clear BSD License
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 * that the following conditions are met:
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
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
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

#ifndef _EMBEDDED_RPC__ARBITRATED_CLIENT_MANAGER_H_
#define _EMBEDDED_RPC__ARBITRATED_CLIENT_MANAGER_H_

#include "client_manager.h"

/*!
 * @addtogroup infra_client
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc {
class TransportArbitrator;

/*!
 * @brief Client that can share a transport with a server.
 *
 * This class is a special subclass of ClientManager that works with the TransportArbitrator
 * to share a single transport with a Server. This enables, for instance, a single UART to
 * be used for bidirectional remote function invocations and replies.
 *
 * The setTransport() method used on ClientManager is not used with this class. Instead, there
 * is a setArbitrator() method. The underlying transport that is shared is set on the arbitrator.
 *
 * @ingroup infra_client
 */
class ArbitratedClientManager : public ClientManager
{
public:
    /*!
     * @brief Constructor.
     *
     * This function initializes object attributes.
     */
    ArbitratedClientManager()
    : ClientManager()
    , m_arbitrator(NULL)
    {
    }

    /*!
     * @brief Sets the transport arbitrator instance.
     *
     * @param[in] arbitrator Transport arbitrator to use.
     */
    void setArbitrator(TransportArbitrator *arbitrator);

protected:
    TransportArbitrator *m_arbitrator; //!< Optional transport arbitrator. May be NULL.

    /*!
     * @brief This function performs request.
     *
     * Should be called in non server context (do not call another eRPC function in server
     * remote call implementation).
     *
     * @param[in] request Request context to perform.
     */
    virtual erpc_status_t performClientRequest(RequestContext &request);

    //! @brief This method is not used with this class.
    void setTransport(Transport *transport) {}
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__ARBITRATED_CLIENT_MANAGER_H_
