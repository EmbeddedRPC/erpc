/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_arbitrated_client_manager.h"

#include "erpc_transport_arbitrator.h"

#include "assert.h"

#if ERPC_THREADS_IS(NONE)
#error "Arbitrator code does not work in no-threading configuration."
#endif

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

#if ERPC_NESTED_CALLS_DETECTION
extern bool nestingDetection;
#ifndef _WIN32
#pragma weak nestingDetection
bool nestingDetection = false;
#endif
#endif

void ArbitratedClientManager::setArbitrator(TransportArbitrator *arbitrator)
{
    m_arbitrator = arbitrator;
    m_transport = arbitrator;
}

void ArbitratedClientManager::performClientRequest(RequestContext &request)
{
    erpc_status_t err;
    TransportArbitrator::client_token_t token = 0;

    assert(m_arbitrator && "arbitrator not set");

    // Set up the client receive before we send the request, so if the reply is sent
    // before we get to the clientReceive() call below the arbitrator already has the buffer.
    if (!request.isOneway())
    {
#if ERPC_NESTED_CALLS_DETECTION
        if (nestingDetection)
        {
            request.getCodec()->updateStatus(kErpcStatus_NestedCallFailure);
        }
#endif
        if (request.getCodec()->isStatusOk() == true)
        {
            token = m_arbitrator->prepareClientReceive(request);
            if (!token)
            {
                request.getCodec()->updateStatus(kErpcStatus_Fail);
            }
        }
    }

#if ERPC_MESSAGE_LOGGING
    if (request.getCodec()->isStatusOk() == true)
    {
        err = logMessage(request.getCodec()->getBuffer());
        request.getCodec()->updateStatus(err);
    }
#endif

    // Send the request.
    if (request.getCodec()->isStatusOk() == true)
    {
        err = m_arbitrator->send(request.getCodec()->getBuffer());
        request.getCodec()->updateStatus(err);
    }

    if (!request.isOneway())
    {
        if (request.getCodec()->isStatusOk() == true)
        {
            // Complete the receive through the arbitrator.
            err = m_arbitrator->clientReceive(token);
            request.getCodec()->updateStatus(err);
        }

#if ERPC_MESSAGE_LOGGING
        if (request.getCodec()->isStatusOk() == true)
        {
            err = logMessage(request.getCodec()->getBuffer());
            request.getCodec()->updateStatus(err);
        }
#endif

        if (request.getCodec()->isStatusOk() == true)
        {
            // Check the reply.
            verifyReply(request);
        }
    }
}
