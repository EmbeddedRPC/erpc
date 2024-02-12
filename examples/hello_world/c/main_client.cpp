/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "hello_world_client.hpp"
#include "examples/config.h"

#include "erpc_basic_codec.hpp"
#include "erpc_client_manager.h"
#include "erpc_tcp_transport.hpp"

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
    TCPTransport transport(ERPC_HOSTNAME, ERPC_PORT, false);
    MyMessageBufferFactory msgFactory;
    BasicCodecFactory basicCodecFactory;
    Crc16 crc16;
    ClientManager clientManager;


    transport.setCrc16(&crc16);
    transport.open();

    clientManager.setMessageBufferFactory(&msgFactory);
    clientManager.setTransport(&transport);
    clientManager.setCodecFactory(&basicCodecFactory);

    /* scope for client service */
    {
        /* init eRPC client TextService service */
        TextService_client client(&clientManager);

        /* do eRPC call */
        client.printText("Hello world!");
    }

    /* deinit objects */
    transport.close();
}
