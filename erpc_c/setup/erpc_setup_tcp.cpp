/*
 * Copyright 2020 (c) Sierra Wireless
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_manually_constructed.h"
#include "erpc_transport_setup.h"
#include "erpc_tcp_transport.h"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

static ManuallyConstructed<TCPTransport> s_transport;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_transport_t erpc_transport_tcp_init_full(const char *host, uint16_t port, bool isServer)
{
    s_transport.construct(host, port, isServer);
    if (kErpcStatus_Success == s_transport->open()) 
    {
        return reinterpret_cast<erpc_transport_t>(s_transport.get());
    }    
    return NULL;    
}

erpc_transport_t erpc_transport_tcp_init(bool isServer)
{
    s_transport.construct(isServer);
    return reinterpret_cast<erpc_transport_t>(s_transport.get());
}

bool erpc_transport_tcp_open(void)
{
    return s_transport.get()->open() == kErpcStatus_Success;
}

void erpc_transport_tcp_close(void)
{
    s_transport.get()->close(true);
}

void erpc_transport_tcp_configure(const char *host, uint16_t port)
{
    s_transport.get()->configure(host, port);
}

