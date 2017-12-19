/*
 * The Clear BSD License
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
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

#include "Logging.h"
#include "basic_codec.h"
#include "client_manager.h"
#include "gtest.h"
#include "gtestListener.h"
#include "myAlloc.h"
#include "tcp_transport.h"
#include "test_unit_test_common.h"

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
        assert(buf);
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

int MyAlloc::allocated_ = 0;

////////////////////////////////////////////////////////////////////////////////
// Set up global fixture - required by BOOST Unit Test Framework
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

    int i = RUN_ALL_TESTS();
    quit();
    free(m_logger);
    g_transport.close();
    free(g_client);

    return i;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
