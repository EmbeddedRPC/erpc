/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_arbitrated_client_manager.h"
#include "erpc_transport_arbitrator.h"
#include "assert.h"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

#if ERPC_NESTED_CALLS_DETECTION
extern bool nestingDetection;
#pragma weak nestingDetection
bool nestingDetection = false;
#endif

void ArbitratedClientManager::setArbitrator(TransportArbitrator *arbitrator)
{
    m_arbitrator = arbitrator;
    m_transport = arbitrator;
}

erpc_status_t ArbitratedClientManager::performClientRequest(RequestContext &request)
{
    assert(m_arbitrator && "arbitrator not set");

    TransportArbitrator::client_token_t token = 0;

    // Set up the client receive before we send the request, so if the reply is sent
    // before we get to the clientReceive() call below the arbitrator already has the buffer.
    if (!request.isOneway())
    {
#if ERPC_NESTED_CALLS_DETECTION
        if (nestingDetection)
        {
            return kErpcStatus_NestedCallFailure;
        }
#endif
        token = m_arbitrator->prepareClientReceive(request);
        if (!token)
        {
            return kErpcStatus_Fail;
        }
    }

    erpc_status_t err;

#if ERPC_MESSAGE_LOGGING
    err = logMessage(request.getCodec()->getBuffer());
    if (err)
    {
        return err;
    }
#endif

    // Send the request.
    err = m_arbitrator->send(request.getCodec()->getBuffer());
    if (err)
    {
        return err;
    }

    if (!request.isOneway())
    {
        // Complete the receive through the arbitrator.
        err = m_arbitrator->clientReceive(token);
        if (err)
        {
            return err;
        }

#if ERPC_MESSAGE_LOGGING
        err = logMessage(request.getCodec()->getBuffer());
        if (err)
        {
            return err;
        }
#endif

        // Check the reply.
        err = verifyReply(request);
        if (err)
        {
            return err;
        }
    }

    return kErpcStatus_Success;
}
