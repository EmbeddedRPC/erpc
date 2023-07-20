/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_basic_codec.hpp"
#include "erpc_client_manager.h"
#include "erpc_tcp_transport.hpp"

#include "Logging.hpp"
#include "c_test_unit_test_common_client.h"
#include "gtest.h"
#include "gtestListener.hpp"
#include "myAlloc.hpp"
#include "unit_test_wrapped.h"

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

MyMessageBufferFactory g_msgFactory;
TCPTransport g_transport("localhost", 12345, false);
#if USE_MESSAGE_LOGGING
TCPTransport g_messageLogger("localhost", 54321, false);
#endif // USE_MESSAGE_LOGGING
BasicCodecFactory g_basicCodecFactory;
ClientManager *g_client;

Crc16 g_crc16;

int ::MyAlloc::allocated_ = 0;

////////////////////////////////////////////////////////////////////////////////
// Set up global fixture
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    ::testing::TestEventListeners &listeners = ::testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new LeakChecker);

    // create logger instance
    StdoutLogger *m_logger = new StdoutLogger();
    m_logger->setFilterLevel(Logger::kInfo);
    Log::setLogger(m_logger);
    Log::info("Starting ERPC client...\n");

    g_client = new ClientManager();
    erpc_status_t err = g_transport.open();
    if (err)
    {
        Log::error("Failed to open connection\n");
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

    g_transport.setCrc16(&g_crc16);
    g_client->setMessageBufferFactory(&g_msgFactory);
    g_client->setTransport(&g_transport);
    g_client->setCodecFactory(&g_basicCodecFactory);
#if USE_MESSAGE_LOGGING
    g_client->addMessageLogger(&g_messageLogger);
#endif // USE_MESSAGE_LOGGING
    erpc_client_t client = reinterpret_cast<erpc_client_t>(g_client);
    initInterfaces_common(client);
    initInterfaces(client);

    int ret = RUN_ALL_TESTS();
    quit();
    free(m_logger);
    g_transport.close();
    free(g_client);

    return ret;
}

void initInterfaces_common(erpc_client_t client)
{
    initCommon_client(client);
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
