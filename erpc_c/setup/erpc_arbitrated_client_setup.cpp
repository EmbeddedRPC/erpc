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

#include "erpc_arbitrated_client_setup.h"
#include "arbitrated_client_manager.h"
#include "manually_constructed.h"
#include "basic_codec.h"
#include "message_buffer.h"
#include "transport_arbitrator.h"
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
static ManuallyConstructed<ArbitratedClientManager> s_client;
ClientManager *g_client;

static ManuallyConstructed<BasicMessageBufferFactory> s_msgFactory;
static ManuallyConstructed<BasicCodecFactory> s_codecFactory;
static ManuallyConstructed<TransportArbitrator> s_arbitrator;
static ManuallyConstructed<BasicCodec> s_codec;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_transport_t erpc_arbitrated_client_init(erpc_transport_t transport)
{
    // Init factories.
    s_msgFactory.construct();
    s_codecFactory.construct();

    // Create codec used by the arbitrator.
    s_codec.construct();

    // Init the arbitrator using the passed in transport.
    s_arbitrator.construct();
    s_arbitrator->setSharedTransport(reinterpret_cast<Transport *>(transport));
    s_arbitrator->setCodec(s_codec);

    // Init the client manager.
    s_client.construct();
    s_client->setArbitrator(s_arbitrator);
    s_client->setMessageBufferFactory(s_msgFactory);
    s_client->setCodecFactory(s_codecFactory);
    g_client = s_client;

    return reinterpret_cast<erpc_transport_t>(s_arbitrator.get());
}

void erpc_arbitrated_client_deinit()
{
    s_client.destroy();
    s_msgFactory.destroy();
    s_codecFactory.destroy();
    s_codec.destroy();
    s_arbitrator.destroy();
}
