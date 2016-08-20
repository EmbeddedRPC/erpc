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

#include "erpc_client_setup.h"
#include "client_manager.h"
#include "manually_constructed.h"
#include "basic_codec.h"
#include "message_buffer.h"
#include "erpc_config_internal.h"
#include <new>
#include <assert.h>

#if !(__embedded_cplusplus)
using namespace std;
#endif

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

class BasicMessageBufferFactory : public MessageBufferFactory
{
public:
    virtual MessageBuffer create()
    {
        uint8_t *buf = new (nothrow) uint8_t[ERPC_DEFAULT_BUFFER_SIZE];
        return MessageBuffer(buf, ERPC_DEFAULT_BUFFER_SIZE);
    }

    virtual void dispose(MessageBuffer *buf)
    {
        assert(buf);
        if (*buf)
        {
            delete[] buf->get();
        }
    }
};

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

// global client variables
static ManuallyConstructed<ClientManager> s_client;
ClientManager *g_client;

static ManuallyConstructed<BasicMessageBufferFactory> s_msgFactory;
static ManuallyConstructed<BasicCodecFactory> s_codecFactory;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

void erpc_client_init(erpc_transport_t transport)
{
    // Init factories.
    s_msgFactory.construct();
    s_codecFactory.construct();

    // Init client manager with the provided transport.
    s_client.construct();
    s_client->setTransport(reinterpret_cast<Transport *>(transport));
    s_client->setMessageBufferFactory(s_msgFactory);
    s_client->setCodecFactory(s_codecFactory);
    g_client = s_client;
}

void erpc_client_set_error_handler(client_error_handler_t error_handler)
{
    if (g_client)
    {
        g_client->setErrorHandler(error_handler);
    }
}

void erpc_client_deinit()
{
    s_client.destroy();
    s_msgFactory.destroy();
    s_codecFactory.destroy();
}
