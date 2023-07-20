/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_arbitrated_client_manager.hpp"
#include "erpc_basic_codec.hpp"
#include "erpc_simple_server.hpp"
#include "erpc_tcp_transport.hpp"
#include "erpc_transport_arbitrator.hpp"

#include "Logging.hpp"
#include "gtest.h"
#include "c_test_firstInterface_client.h"
#include "c_test_unit_test_common_client.h"
#include "unit_test_wrapped.h"
#include "unit_test.h"

#include <chrono>
#include <thread>
#include <unistd.h>

using namespace erpc;

class MyMessageBufferFactory : public MessageBufferFactory
{
public:
    virtual MessageBuffer create()
    {
        uint8_t *buf = new uint8_t[1024];
        return MessageBuffer(buf, 1024);
    }

    virtual void dispose(MessageBuffer *buf)
    {
        erpc_assert(buf);
        if (*buf)
        {
            delete[] buf->get();
        }
    }
};

TCPTransport g_transport("localhost", 12345, false);
#if USE_MESSAGE_LOGGING
TCPTransport g_messageLogger("localhost", 54321, false);
#endif // USE_MESSAGE_LOGGING
MyMessageBufferFactory g_msgFactory;
BasicCodecFactory g_basicCodecFactory;
ArbitratedClientManager *g_client;
TransportArbitrator g_arbitrator;
SimpleServer g_server;
Mutex waitQuitMutex;

extern const uint32_t erpc_generated_crc;
Crc16 g_crc16(erpc_generated_crc);

int waitQuit = 0;

void increaseWaitQuit()
{
    Mutex::Guard lock(waitQuitMutex);
    waitQuit++;
    Mutex::Guard unlock(waitQuitMutex);
}

void runServer(void *arg)
{
    erpc_status_t *err = (erpc_status_t *)arg;
    *err = g_server.run();
    increaseWaitQuit();
}

void runClient(void *arg)
{
    // send to ERPC second (server) app message that this app is ready.
    whenReady();

    int *i = (int *)arg;
    *i = RUN_ALL_TESTS();

    // wait until ERPC second (server) app will announce ready to quit state.
    while (true)
    {
        Mutex::Guard lock(waitQuitMutex);
        if (waitQuit != 0)
        {
            break;
        }
        Mutex::Guard unlock(waitQuitMutex);
    }

    // send to ERPC second (server) app ready to quit state
    quitFirstInterfaceServer();

    increaseWaitQuit();
}

////////////////////////////////////////////////////////////////////////////////
// Set up global fixture
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    // create logger instance
    StdoutLogger m_logger;

    m_logger.setFilterLevel(Logger::kInfo);
    Log::setLogger(&m_logger);

    Log::info("Intit ERPC first (client) app...\n");

    std::chrono::milliseconds duration(500);
    std::this_thread::sleep_for(duration);
    erpc_status_t err = g_transport.open();
    if (err)
    {
        Log::error("Failed to open connection in ERPC first (client) app\n");
        return err;
    }

#if USE_MESSAGE_LOGGING
    g_messageLogger.setCrc16(&g_crc16);
    err = g_messageLogger.open();
    if (err)
    {
        Log::error("Failed to open connection in ERPC first (client) app\n");
        return err;
    }
#endif // USE_MESSAGE_LOGGING

    g_arbitrator.setSharedTransport(&g_transport);
    g_arbitrator.setCodec(g_basicCodecFactory.create());

    g_client = new ArbitratedClientManager();
    g_client->setArbitrator(&g_arbitrator);
    g_client->setCodecFactory(&g_basicCodecFactory);
    g_client->setMessageBufferFactory(&g_msgFactory);
#if USE_MESSAGE_LOGGING
    g_client->addMessageLogger(&g_messageLogger);
#endif // USE_MESSAGE_LOGGING

    g_arbitrator.setCrc16(&g_crc16);

    g_server.setTransport(&g_arbitrator);
    g_server.setCodecFactory(&g_basicCodecFactory);
    g_server.setMessageBufferFactory(&g_msgFactory);
#if USE_MESSAGE_LOGGING
    g_server.addMessageLogger(&g_messageLogger);
#endif // USE_MESSAGE_LOGGING

    add_services(&g_server);
    g_client->setServer(&g_server);
    erpc_client_t client = reinterpret_cast<erpc_client_t>(g_client);
    initInterfaces_common(client);
    initInterfaces(client);

    int i = -1;
    err = (erpc_status_t)-1;

    // One thread for client and one for server part of application
    Thread serverThread(&runServer, 0, 0, "server");
    serverThread.start(&err);
    g_client->setServerThreadId(serverThread.getThreadId());
    Thread clientThread(&runClient, 0, 0, "client");
    clientThread.start(&i);

    // Wait until server and client will stop.
    while (true)
    {
        Mutex::Guard lock(waitQuitMutex);
        if (waitQuit >= 3)
        {
            break;
        }
        Mutex::Guard unlock(waitQuitMutex);
    }

    // Close transport
    g_transport.close();

    free(g_client);

    if (err && err != kErpcStatus_ServerIsDown)
    {
        Log::error("Error occurred in ERPC first (client) app: %d\n", err);
        return err;
    }

    return i;
}

void quitSecondInterfaceServer()
{
    // removing SecondInterface service from the server
    remove_services(&g_server);
    // Stop server part
    g_server.stop();
    increaseWaitQuit();
}

void initInterfaces_common(erpc_client_t client)
{
    initCommon_client(client);
}
