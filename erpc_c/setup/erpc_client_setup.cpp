/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
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

#include "erpc_client_setup.h"
#include "basic_codec.h"
#include "client_manager.h"
#include "crc16.h"
#include "manually_constructed.h"
#include <assert.h>
#include <new>

#if !(__embedded_cplusplus)
using namespace std;
#endif

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

// global client variables
static ManuallyConstructed<ClientManager> s_client;
ClientManager *g_client;
static ManuallyConstructed<BasicCodecFactory> s_codecFactory;
static ManuallyConstructed<Crc16> s_crc16;

extern const uint32_t erpc_generated_crc;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

void erpc_client_init(erpc_transport_t transport, erpc_mbf_t message_buffer_factory)
{
    // Init factories.
    s_codecFactory.construct();

    // Init client manager with the provided transport.
    s_client.construct();
    Transport *castedTransport = reinterpret_cast<Transport *>(transport);
    s_crc16.construct(erpc_generated_crc);
    castedTransport->setCrc16(s_crc16.get());
    s_client->setTransport(castedTransport);
    s_client->setCodecFactory(s_codecFactory);
    s_client->setMessageBufferFactory(reinterpret_cast<MessageBufferFactory *>(message_buffer_factory));
    g_client = s_client;
}

void erpc_client_set_error_handler(client_error_handler_t error_handler)
{
    if (g_client)
    {
        g_client->setErrorHandler(error_handler);
    }
}

#if ERPC_NESTED_CALLS
void erpc_client_set_server(erpc_server_t server)
{
    g_client->setServer(reinterpret_cast<Server *>(server));
}
#endif

#if ERPC_MESSAGE_LOGGING
void erpc_server_add_message_logger(erpc_transport_t transport)
{
    g_client->addMessageLogger(reinterpret_cast<Transport *>(transport));
}
#endif

void erpc_client_deinit()
{
    s_crc16.destroy();
    s_client.destroy();
    s_codecFactory.destroy();
}
