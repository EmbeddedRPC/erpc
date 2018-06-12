/*
 * The Clear BSD License
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
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

#include "erpc_server_setup.h"
#include "erpc_basic_codec.h"
#include "erpc_crc16.h"
#include "erpc_manually_constructed.h"
#include "erpc_message_buffer.h"
#include "erpc_simple_server.h"
#include "erpc_transport.h"
#include <cassert>

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

// global server variables
static ManuallyConstructed<SimpleServer> s_server;
SimpleServer *g_server = NULL;
static ManuallyConstructed<BasicCodecFactory> s_codecFactory;
static ManuallyConstructed<Crc16> s_crc16;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_server_t erpc_server_init(erpc_transport_t transport, erpc_mbf_t message_buffer_factory)
{
    assert(transport);

    // Init factories.
    s_codecFactory.construct();

    // Init server with the provided transport.
    s_server.construct();
    Transport *castedTransport = reinterpret_cast<Transport *>(transport);
    s_crc16.construct();
    castedTransport->setCrc16(s_crc16.get());
    s_server->setTransport(castedTransport);
    s_server->setCodecFactory(s_codecFactory);
    s_server->setMessageBufferFactory(reinterpret_cast<MessageBufferFactory *>(message_buffer_factory));
    g_server = s_server;
    return reinterpret_cast<erpc_server_t>(g_server);
}

void erpc_server_deinit(void)
{
    s_crc16.destroy();
    s_codecFactory.destroy();
    s_server.destroy();
    g_server = NULL;
}

void erpc_add_service_to_server(void *service)
{
    if (g_server != NULL && service != NULL)
    {
        g_server->addService(static_cast<erpc::Service *>(service));
    }
}

void erpc_server_set_crc(uint32_t crcStart)
{
    s_crc16->setCrcStart(crcStart);
}

erpc_status_t erpc_server_run(void)
{
    if (g_server != NULL)
    {
        return g_server->run();
    }
    return kErpcStatus_Fail;
}

erpc_status_t erpc_server_poll(void)
{
    if (g_server != NULL)
    {
        return g_server->poll();
    }
    return kErpcStatus_Fail;
}

void erpc_server_stop(void)
{
    if (g_server != NULL)
    {
        g_server->stop();
    }
}

#if ERPC_MESSAGE_LOGGING
bool erpc_server_add_message_logger(erpc_transport_t transport)
{
    if (g_server != NULL)
    {
        return g_server->addMessageLogger(reinterpret_cast<Transport *>(transport));
    }
    return false;
}
#endif
