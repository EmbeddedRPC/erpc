/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "hello_world_server.hpp"
#include "config.h"

#include "erpc_basic_codec.hpp"
#include "erpc_crc16.hpp"
#include "erpc_message_buffer.hpp"
#include "erpc_simple_server.hpp"
#include "erpc_tcp_transport.hpp"

#include "erpc_error_handler.h"

#include <iostream>

using namespace erpcShim;
using namespace erpc;

bool g_server_run = true;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

class MyMessageBufferFactory : public MessageBufferFactory
{
public:
    virtual MessageBuffer create()
    {
        uint8_t *buf = new uint8_t[ERPC_DEFAULT_BUFFER_SIZE];
        return MessageBuffer(buf, ERPC_DEFAULT_BUFFER_SIZE);
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

class TextService : public TextService_interface
{
    bool printText(const char *text)
    {
        std::cout << text << std::endl;
        return true;
    }

    void stopServer() { g_server_run = false; }
};

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

int main()
{
    erpc_status_t status;

    /* Init eRPC server components */
    TCPTransport transport(ERPC_HOSTNAME, ERPC_PORT, true);
    MyMessageBufferFactory msgFactory;
    BasicCodecFactory codecFactory;
    SimpleServer server;
    Crc16 crc16;

    /* Init service implementation and handler */
    TextService textServiceImpl;
    TextService_service textService(&textServiceImpl);

    /* Setup transport */
    transport.setCrc16(&crc16);
    status = transport.open();

    if (status != kErpcStatus_Success)
    {
        /* Print error description */
        erpc_error_handler(status, 0);
        return -1;
    }

    /* Setup server */
    server.setTransport(&transport);
    server.setCodecFactory(&codecFactory);
    server.setMessageBufferFactory(&msgFactory);

    /* add custom service implementation to the server */
    server.addService(&textService);

    std::cout << "Starting server." << std::endl;

    while (g_server_run)
    {
        /* poll for requests */
        status = server.poll();

        /* handle error status */
        if (status != kErpcStatus_Success)
        {
            /* print error description */
            erpc_error_handler(status, 0);
        }
    }

    std::cout << "Server stopped." << std::endl;

    /* Close transport */
    transport.close();
}