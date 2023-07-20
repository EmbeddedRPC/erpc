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
#include "c_test_secondInterface_server.h"
#include "myAlloc.hpp"
#include "unit_test.h"

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

int testClient();

TCPTransport g_transport("localhost", 12345, true);
MyMessageBufferFactory g_msgFactory;
BasicCodecFactory g_basicCodecFactory;
ArbitratedClientManager *g_client;
TransportArbitrator g_arbitrator;
SimpleServer g_server;
Mutex waitQuitMutex;

extern const uint32_t erpc_generated_crc;
Crc16 g_crc16(erpc_generated_crc);

volatile int waitQuit = 0;
volatile int waitClient = 0;
volatile int isTestPassing = 0;
volatile int stopTest = 0;

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
    // wait until ERPC first (client) app will announce that it is ready.
    while (waitClient == 0)
    {
    }

    // wait until ERPC first (client) app will announce ready to quit state
    while (true)
    {
        isTestPassing = testClient();
        Mutex::Guard lock(waitQuitMutex);
        if (waitQuit != 0 || isTestPassing != 0 || stopTest != 0)
        {
            enableFirstSide();
            Mutex::Guard unlock(waitQuitMutex);
            break;
        }
        Mutex::Guard unlock(waitQuitMutex);
    }

    while (true)
    {
        Mutex::Guard lock(waitQuitMutex);
        if (waitQuit != 0)
        {
            break;
        }
        Mutex::Guard unlock(waitQuitMutex);
    }

    // send to ERPC first (client) app ready to quit state
    quitSecondInterfaceServer();

    increaseWaitQuit();
}

////////////////////////////////////////////////////////////////////////////////
// Set up global fixture
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    // create logger instance
    StdoutLogger m_logger;

    m_logger.setFilterLevel(Logger::kInfo);

    Log::setLogger(&m_logger);

    Log::info("Intit ERPC second (server) app...\n");

    erpc_status_t err = g_transport.open();
    if (err)
    {
        Log::error("Failed to open connection in ERPC second (server) app\n");
        return err;
    }

    g_arbitrator.setSharedTransport(&g_transport);
    g_arbitrator.setCodec(g_basicCodecFactory.create());

    g_client = new ArbitratedClientManager();
    g_client->setArbitrator(&g_arbitrator);
    g_client->setCodecFactory(&g_basicCodecFactory);
    g_client->setMessageBufferFactory(&g_msgFactory);

    g_arbitrator.setCrc16(&g_crc16);

    g_server.setTransport(&g_arbitrator);
    g_server.setCodecFactory(&g_basicCodecFactory);
    g_server.setMessageBufferFactory(&g_msgFactory);
    add_services(&g_server);
    g_client->setServer(&g_server);

    err = (erpc_status_t)-1;

    // One thread for client and one for server app of application
    Thread serverThread(&runServer, 0, 0, "server");
    serverThread.start(&err);
    g_client->setServerThreadId(serverThread.getThreadId());
    Thread clientThread(&runClient, 0, 0, "client");
    clientThread.start(&clientThread);

    // Wait until client side will stop.
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
        Log::error("Error occurred in ERPC second (server) app: %d\n", err);
        return err;
    }

    return isTestPassing;
}

extern "C" {
void stopSecondSide()
{
    ++stopTest;
}

int32_t getResultFromSecondSide()
{
    return isTestPassing;
}

void testCasesAreDone(void)
{
    increaseWaitQuit();
}

void quitFirstInterfaceServer()
{
    // removing FirstInterface service from the server
    remove_services(&g_server);
    // Stop server part
    g_server.stop();
}

void whenReady()
{
    waitClient++;
}
}

int testClient()
{
    int number = 15;
    for (int i = 0; i < number; i++)
    {
        secondSendInt(i + number);
    }
    for (int i = number - 1; i >= 0; i--)
    {
        if (i + number != secondReceiveInt())
        {
            return -1;
        }
    }
    return 0;
}
