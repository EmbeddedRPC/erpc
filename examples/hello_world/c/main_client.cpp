/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "hello_world_client.hpp"
#include "config.h"

#include "erpc_basic_codec.hpp"
#include "erpc_client_manager.h"
#include "erpc_tcp_transport.hpp"

#include "erpc_error_handler.h"

#include <iostream>

using namespace erpcShim;
using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

int main()
{
    erpc_status_t status;

    /* Init eRPC client components */
    TCPTransport transport(ERPC_HOSTNAME, ERPC_PORT, false);
    MyMessageBufferFactory msgFactory;
    BasicCodecFactory basicCodecFactory;
    Crc16 crc16;
    ClientManager clientManager;

    /* Init transport */
    transport.setCrc16(&crc16);
    status = transport.open();

    if (status != kErpcStatus_Success)
    {
        /* print error description */
        erpc_error_handler(status, 0);
        return -1;
    }

    /* Init client manager */
    clientManager.setMessageBufferFactory(&msgFactory);
    clientManager.setTransport(&transport);
    clientManager.setCodecFactory(&basicCodecFactory);

    /* Scope for client service */
    {
        /* Init eRPC client TextService service */
        TextService_client client(&clientManager);

        /* Do eRPC call */
        if (client.printText("Hello world!"))
        {
            std::cout << "Message received by server." << std::endl;
        }

        /* Stop server */
        client.stopServer();
    }

    /* Deinit objects */
    transport.close();
}