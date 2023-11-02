/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * Copyright 2020-2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_arbitrated_client_setup.h"

#include "erpc_arbitrated_client_manager.hpp"
#include "erpc_basic_codec.hpp"
#include "erpc_manually_constructed.hpp"
#include "erpc_message_buffer.hpp"
#include "erpc_transport_arbitrator.hpp"
#if ERPC_NESTED_CALLS
#include "erpc_threading.h"
#endif

#if ERPC_THREADS_IS(NONE)
#error "Arbitrator code does not work in no-threading configuration."
#endif

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

// global client variables
ERPC_MANUALLY_CONSTRUCTED_STATIC(ArbitratedClientManager, s_client);
#if defined(__MINGW32__)
__declspec(selectany)
#endif
    ClientManager *g_client;
#if !defined(__MINGW32__)
#pragma weak g_client
#endif

ERPC_MANUALLY_CONSTRUCTED_STATIC(BasicCodecFactory, s_codecFactory);
ERPC_MANUALLY_CONSTRUCTED_STATIC(TransportArbitrator, s_arbitrator);
ERPC_MANUALLY_CONSTRUCTED_STATIC(BasicCodec, s_codec);
ERPC_MANUALLY_CONSTRUCTED_STATIC(Crc16, s_crc16);

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_client_t erpc_arbitrated_client_init(erpc_transport_t transport, erpc_mbf_t message_buffer_factory,
                                          erpc_transport_t *arbitrator)
{
    erpc_assert(transport != NULL);
    erpc_assert(message_buffer_factory != NULL);
    erpc_assert(arbitrator != NULL);

    Transport *castedTransport;
    BasicCodecFactory *codecFactory;
    BasicCodec *codec;
    TransportArbitrator *transportArbitrator;
    Crc16 *crc16;
    ArbitratedClientManager *client;

#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    if (s_codecFactory.isUsed() || s_codec.isUsed() || s_arbitrator.isUsed() || s_crc16.isUsed() || s_client.isUsed())
    {
        client = NULL;
    }
    else
    {
        // Init factories.
        s_codecFactory.construct();
        codecFactory = s_codecFactory.get();

        // Create codec used by the arbitrator.
        s_codec.construct();
        codec = s_codec.get();

        // Init the arbitrator using the passed in transport.
        s_arbitrator.construct();
        transportArbitrator = s_arbitrator.get();

        s_crc16.construct();
        crc16 = s_crc16.get();

        // Init the client manager.
        s_client.construct();
        client = s_client.get();
    }
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    // Init factories.
    codecFactory = new BasicCodecFactory();

    // Create codec used by the arbitrator.
    codec = new BasicCodec();

    // Init the arbitrator using the passed in transport.
    transportArbitrator = new TransportArbitrator();

    crc16 = new Crc16();

    // Init the client manager.
    client = new ArbitratedClientManager();

    if ((codecFactory == NULL) || (codec == NULL) || (transportArbitrator == NULL) || (crc16 == NULL) ||
        (client == NULL))
    {
        if (codecFactory != NULL)
        {
            delete codecFactory;
        }
        if (codec != NULL)
        {
            delete codec;
        }
        if (transportArbitrator != NULL)
        {
            delete transportArbitrator;
        }
        if (crc16 != NULL)
        {
            delete crc16;
        }
        if (client != NULL)
        {
            delete client;
        }
        client = NULL;
    }

#else
#error "Unknown eRPC allocation policy!"
#endif

    if (client == NULL)
    {
        *arbitrator = NULL;
    }
    else
    {
        castedTransport = reinterpret_cast<Transport *>(transport);
        castedTransport->setCrc16(crc16);
        transportArbitrator->setSharedTransport(castedTransport);
        transportArbitrator->setCodec(codec);

        client->setArbitrator(transportArbitrator);
        client->setCodecFactory(codecFactory);
        client->setMessageBufferFactory(reinterpret_cast<MessageBufferFactory *>(message_buffer_factory));

        *arbitrator = reinterpret_cast<erpc_transport_t>(transportArbitrator);
    }

    g_client = dynamic_cast<ClientManager *>(client);

    return reinterpret_cast<erpc_client_t>(client);
}

void erpc_arbitrated_client_set_error_handler(erpc_client_t client, client_error_handler_t error_handler)
{
    erpc_assert(client != NULL);

    ArbitratedClientManager *clientManager = reinterpret_cast<ArbitratedClientManager *>(client);

    clientManager->setErrorHandler(error_handler);
}

void erpc_arbitrated_client_set_crc(erpc_client_t client, uint32_t crcStart)
{
    erpc_assert(client != NULL);

    ArbitratedClientManager *clientManager = reinterpret_cast<ArbitratedClientManager *>(client);

    clientManager->getArbitrator()->getSharedTransport()->getCrc16()->setCrcStart(crcStart);
}

#if ERPC_NESTED_CALLS
void erpc_arbitrated_client_set_server(erpc_client_t client, erpc_server_t server)
{
    erpc_assert(client != NULL);

    ArbitratedClientManager *clientManager = reinterpret_cast<ArbitratedClientManager *>(client);

    clientManager->setServer(reinterpret_cast<Server *>(server));
}

void erpc_arbitrated_client_set_server_thread_id(erpc_client_t client, void *serverThreadId)
{
    erpc_assert(client != NULL);

    ArbitratedClientManager *clientManager = reinterpret_cast<ArbitratedClientManager *>(client);

    clientManager->setServerThreadId(reinterpret_cast<Thread::thread_id_t *>(serverThreadId));
}
#endif

#if ERPC_MESSAGE_LOGGING
bool erpc_arbitrated_client_add_message_logger(erpc_client_t client, erpc_transport_t transport)
{
    erpc_assert(client != NULL);

    ArbitratedClientManager *clientManager = reinterpret_cast<ArbitratedClientManager *>(client);

    return clientManager->addMessageLogger(reinterpret_cast<Transport *>(transport));
}
#endif

#if ERPC_PRE_POST_ACTION
void erpc_arbitrated_client_add_pre_cb_action(erpc_client_t client, pre_post_action_cb preCB)
{
    erpc_assert(client != NULL);

    ArbitratedClientManager *clientManager = reinterpret_cast<ArbitratedClientManager *>(client);

    clientManager->addPreCB(preCB);
}

void erpc_arbitrated_client_add_post_cb_action(erpc_client_t client, pre_post_action_cb postCB)
{
    erpc_assert(client != NULL);

    ArbitratedClientManager *clientManager = reinterpret_cast<ArbitratedClientManager *>(client);

    clientManager->addPostCB(postCB);
}
#endif

void erpc_arbitrated_client_deinit(erpc_client_t client)
{
#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    (void)client;
    erpc_assert(reinterpret_cast<ArbitratedClientManager *>(client) == s_client.get());
    s_codecFactory.destroy();
    s_crc16.destroy();
    s_codec.destroy();
    s_arbitrator.destroy();
    s_client.destroy();
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    erpc_assert(client != NULL);
    ArbitratedClientManager *clientManager = reinterpret_cast<ArbitratedClientManager *>(client);
    erpc::TransportArbitrator *arbitrator = clientManager->getArbitrator();
    delete clientManager->getCodecFactory();
    delete arbitrator->getSharedTransport()->getCrc16();
    delete arbitrator->getCodec();
    delete arbitrator;
    delete clientManager;
#endif
}
