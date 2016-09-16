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

#include "erpc_server_setup.h"
#include "simple_server.h"
#include "manually_constructed.h"
#include "basic_codec.h"
#include "erpc_setup.h"
#include <new>
#include <assert.h>

#if !(__embedded_cplusplus)
using namespace std;
#endif

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

// global server variables
static ManuallyConstructed<SimpleServer> s_server;
SimpleServer *g_server;

static ManuallyConstructed<BasicMessageBufferFactory> s_msgFactory;
static ManuallyConstructed<BasicCodecFactory> s_codecFactory;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

void erpc_server_init(erpc_transport_t transport)
{
    // Init factories.
    s_msgFactory.construct();
    s_codecFactory.construct();

    // Init server with the provided transport.
    s_server.construct();
    s_server->setTransport(reinterpret_cast<Transport *>(transport));
    s_server->setMessageBufferFactory(s_msgFactory);
    s_server->setCodecFactory(s_codecFactory);
    g_server = s_server;
}

void erpc_server_deinit()
{
    s_msgFactory.destroy();
    s_codecFactory.destroy();
    s_server.destroy();
}

void erpc_add_service_to_server(void *service)
{
    if (service != NULL)
    {
        g_server->addService(static_cast<erpc::Service *>(service));
    }
}

status_t erpc_server_run()
{
    return g_server->run();
}

status_t erpc_server_poll()
{
    return g_server->poll();
}

void erpc_server_stop()
{
    g_server->stop();
}
