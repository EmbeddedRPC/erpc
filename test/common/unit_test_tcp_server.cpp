/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_basic_codec.h"
#include "erpc_simple_server.h"
#include "erpc_tcp_transport.h"
#include "Logging.h"
#include "myAlloc.h"
#include "test_unit_test_common_server.h"
#include "unit_test.h"

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

TCPTransport g_transport("localhost", 12345, true);
MyMessageBufferFactory g_msgFactory;
BasicCodecFactory g_basicCodecFactory;
SimpleServer g_server;

Crc16 g_crc16;

int MyAlloc::allocated_ = 0;

Common_service *svc_common;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////
int main(int argc, const char *argv[])
{
    // create logger instance
    StdoutLogger *m_logger = new StdoutLogger();
    m_logger->setFilterLevel(Logger::kInfo);
    Log::setLogger(m_logger);
    Log::info("Starting ERPC server...\n");

    erpc_status_t result = g_transport.open();
    if (result)
    {
        Log::error("Failed to open connection\n");
        return 1;
    }

    g_transport.setCrc16(&g_crc16);
    g_server.setMessageBufferFactory(&g_msgFactory);
    g_server.setTransport(&g_transport);
    g_server.setCodecFactory(&g_basicCodecFactory);

    add_services(&g_server);
    add_common_service(&g_server);
    // run server infinitely
    erpc_status_t err = g_server.run();
    if (err && err != kErpcStatus_ServerIsDown)
    {
        Log::error("Error occurred: %d\n", err);
        return err;
    }

    //    Thread::sleep(10000000);
    free(m_logger);
    g_transport.close();

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Server helper functions
////////////////////////////////////////////////////////////////////////////////
void add_common_service(SimpleServer *server)
{
    svc_common = new Common_service();

    server->addService(svc_common);
}

void remove_common_service(SimpleServer *server)
{
    server->removeService(svc_common);
    delete svc_common;
}

extern "C" void erpc_add_service_to_server(void *service) {}
extern "C" void erpc_remove_service_from_server(void *service) {}

////////////////////////////////////////////////////////////////////////////////
// Common service implementations here
////////////////////////////////////////////////////////////////////////////////
void quit()
{
    remove_common_service(&g_server);
    remove_services(&g_server);
    g_server.stop();
}

int32_t getServerAllocated()
{
    int result = MyAlloc::allocated();
    MyAlloc::allocated(0);
    return result;
}
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
