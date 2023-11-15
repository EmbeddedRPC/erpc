/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__ARBITRATED_CLIENT_MANAGER_H_
#define _EMBEDDED_RPC__ARBITRATED_CLIENT_MANAGER_H_

#include "erpc_client_manager.h"

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
    ArbitratedClientManager(void) : ClientManager(), m_arbitrator(NULL) {}

    /*!
     * @brief Sets the transport arbitrator instance.
     *
     * @param[in] arbitrator Transport arbitrator to use.
     */
    void setArbitrator(TransportArbitrator *arbitrator);

    /*!
     * @brief Gets the transport arbitrator instance.
     *
     * @return TransportArbitrator * Transport arbitrator instance.
     */
    TransportArbitrator *getArbitrator(void) { return m_arbitrator; };

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
    virtual void performClientRequest(RequestContext &request) override;

    //! @brief This method is not used with this class.
    void setTransport(Transport *transport) { (void)transport; }
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__ARBITRATED_CLIENT_MANAGER_H_
