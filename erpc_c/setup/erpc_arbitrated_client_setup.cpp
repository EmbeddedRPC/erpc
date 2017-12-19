/*
 * The Clear BSD License
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
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

#include "erpc_arbitrated_client_setup.h"
#include "arbitrated_client_manager.h"
#include "basic_codec.h"
#include "manually_constructed.h"
#include "message_buffer.h"
#include "transport_arbitrator.h"
#if ERPC_NESTED_CALLS
#include "erpc_threading.h"
#endif
#include <cassert>

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

// global client variables
static ManuallyConstructed<ArbitratedClientManager> s_client;
ClientManager *g_client = NULL;

static ManuallyConstructed<BasicCodecFactory> s_codecFactory;
static ManuallyConstructed<TransportArbitrator> s_arbitrator;
static ManuallyConstructed<BasicCodec> s_codec;
static ManuallyConstructed<Crc16> s_crc16;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_transport_t erpc_arbitrated_client_init(erpc_transport_t transport, erpc_mbf_t message_buffer_factory)
{
    assert(transport);

    // Init factories.
    s_codecFactory.construct();

    // Create codec used by the arbitrator.
    s_codec.construct();

    // Init the arbitrator using the passed in transport.
    s_arbitrator.construct();
    Transport *castedTransport = reinterpret_cast<Transport *>(transport);
    s_crc16.construct();
    castedTransport->setCrc16(s_crc16.get());
    s_arbitrator->setSharedTransport(castedTransport);
    s_arbitrator->setCodec(s_codec);

    // Init the client manager.
    s_client.construct();
    s_client->setArbitrator(s_arbitrator);
    s_client->setCodecFactory(s_codecFactory);
    s_client->setMessageBufferFactory(reinterpret_cast<MessageBufferFactory *>(message_buffer_factory));
    g_client = s_client;

    return reinterpret_cast<erpc_transport_t>(s_arbitrator.get());
}

void erpc_arbitrated_client_set_error_handler(client_error_handler_t error_handler)
{
    if (g_client != NULL)
    {
        g_client->setErrorHandler(error_handler);
    }
}

void erpc_arbitrated_client_set_crc(uint32_t crcStart)
{
    s_crc16->setCrcStart(crcStart);
}

#if ERPC_NESTED_CALLS
void erpc_arbitrated_client_set_server(erpc_server_t server)
{
    if (g_client != NULL)
    {
        g_client->setServer(reinterpret_cast<Server *>(server));
    }
}

void erpc_arbitrated_client_set_server_thread_id(void *serverThreadId)
{
    if (g_client != NULL)
    {
        g_client->setServerThreadId(reinterpret_cast<Thread::thread_id_t *>(serverThreadId));
    }
}
#endif

#if ERPC_MESSAGE_LOGGING
bool erpc_arbitrated_client_add_message_logger(erpc_transport_t transport)
{
    if (g_client != NULL)
    {
        return g_client->addMessageLogger(reinterpret_cast<Transport *>(transport));
    }
    return false;
}
#endif

void erpc_arbitrated_client_deinit()
{
    s_client.destroy();
    s_codecFactory.destroy();
    s_codec.destroy();
    s_arbitrator.destroy();
    g_client = NULL;
}
