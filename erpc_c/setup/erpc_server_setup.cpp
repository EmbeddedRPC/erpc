/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * Copyright 2020-2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_server_setup.h"
#include "erpc_basic_codec.h"
#include "erpc_crc16.h"
#include "erpc_manually_constructed.h"
#include "erpc_message_buffer.h"
#include "erpc_simple_server.h"
#include "erpc_transport.h"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

// global server variables
ERPC_MANUALLY_CONSTRUCTED(SimpleServer, s_server);
extern SimpleServer *g_server;
SimpleServer *g_server = NULL;
ERPC_MANUALLY_CONSTRUCTED(BasicCodecFactory, s_codecFactory);
ERPC_MANUALLY_CONSTRUCTED(Crc16, s_crc16);

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_server_t erpc_server_init(erpc_transport_t transport, erpc_mbf_t message_buffer_factory)
{
    erpc_assert(transport);

    Transport *castedTransport;

    // Init factories.
    s_codecFactory.construct();

    // Init server with the provided transport.
    s_server.construct();
    castedTransport = reinterpret_cast<Transport *>(transport);
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
    if ((g_server != NULL) && (service != NULL))
    {
        g_server->addService(static_cast<erpc::Service *>(service));
    }
}

void erpc_remove_service_from_server(void *service)
{
    if ((g_server != NULL) && (service != NULL))
    {
        g_server->removeService(static_cast<erpc::Service *>(service));
    }
}

void erpc_server_set_crc(uint32_t crcStart)
{
    s_crc16->setCrcStart(crcStart);
}

erpc_status_t erpc_server_run(void)
{
    erpc_status_t status;

    if (g_server == NULL)
    {
        status = kErpcStatus_Fail;
    }
    else
    {
        status = g_server->run();
    }

    return status;
}

erpc_status_t erpc_server_poll(void)
{
    erpc_status_t status;

    if (g_server == NULL)
    {
        status = kErpcStatus_Fail;
    }
    else
    {
        status = g_server->poll();
    }

    return status;
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
    bool retVal;

    if (g_server == NULL)
    {
        retVal = false;
    }
    else
    {
        retVal = g_server->addMessageLogger(reinterpret_cast<Transport *>(transport));
    }

    return retVal;
}
#endif

#if ERPC_PRE_POST_ACTION
void erpc_client_add_pre_cb_action(pre_post_action_cb preCB)
{
    erpc_assert(g_server);

    g_server->addPreCB(preCB);
}

void erpc_client_add_post_cb_action(pre_post_action_cb postCB)
{
    erpc_assert(g_server);

    g_server->addPostCB(postCB);
}
#endif
