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

#include "arbitrated_client_manager.h"
#include "assert.h"
#include "transport_arbitrator.h"

#if !(__embedded_cplusplus)
using namespace std;
#endif

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

void ArbitratedClientManager::setArbitrator(TransportArbitrator *arbitrator)
{
    m_arbitrator = arbitrator;
    m_transport = arbitrator;
}

erpc_status_t ArbitratedClientManager::performRequest(RequestContext &request)
{
    assert(m_arbitrator && "arbitrator not set");

    TransportArbitrator::client_token_t token = 0;

    // Set up the client receive before we send the request, so if the reply is sent
    // before we get to the clientReceive() call below the arbitrator already has the buffer.
    if (!request.isOneway())
    {
        token = m_arbitrator->prepareClientReceive(request);
        if (!token)
        {
            return kErpcStatus_Fail;
        }
    }

    // Send the request.
    erpc_status_t err = m_arbitrator->send(request.getCodec()->getBuffer());
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

        err = verifyReply(request);
        if (err)
        {
            return err;
        }
    }

    return kErpcStatus_Success;
}
