/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_basic_codec.hpp"
#include "erpc_client_manager.h"
#include "erpc_serial_transport.hpp"

#include "Logging.hpp"
#include "c_test_unit_test_common_client.h"
#include "gtest.h"
#include "gtestListener.hpp"
#include "myAlloc.hpp"
#include "unit_test_wrapped.h"

////////////////////////////////////////////////////////////////////////////////
// DEFINITIONS
////////////////////////////////////////////////////////////////////////////////
#ifndef UNIT_TEST_SERIAL_PORT
#define UNIT_TEST_SERIAL_PORT "/dev/ttyS4"
#endif

#ifndef UNIT_TEST_SERIAL_BAUD
#define UNIT_TEST_SERIAL_BAUD 115200
#endif

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////
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

SerialTransport g_transport(UNIT_TEST_SERIAL_PORT, UNIT_TEST_SERIAL_BAUD);
MyMessageBufferFactory g_msgFactory;
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
    m_logger->setFilterLevel(Logger::log_level_t::kInfo);
    Log::setLogger(m_logger);
    Log::info("Starting ERPC client on port '%s' with baud %d.\n", UNIT_TEST_SERIAL_PORT, UNIT_TEST_SERIAL_BAUD);

    g_client = new ClientManager();
    uint8_t vtime = 0;
    uint8_t vmin = 1;
    while (kErpcStatus_Success != g_transport.init(vtime, vmin))
        ;

    g_transport.setCrc16(&g_crc16);
    g_client->setMessageBufferFactory(&g_msgFactory);
    g_client->setTransport(&g_transport);
    g_client->setCodecFactory(&g_basicCodecFactory);
    erpc_client_t client = reinterpret_cast<erpc_client_t>(g_client);
    initInterfaces_common(client);
    initInterfaces(client);

    int ret = RUN_ALL_TESTS();
    quit();
    free(m_logger);
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
