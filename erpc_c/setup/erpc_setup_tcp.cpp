/*
 * Copyright 2020 (c) Sierra Wireless
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_manually_constructed.h"
#include "erpc_tcp_transport.h"
#include "erpc_transport_setup.h"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

static ManuallyConstructed<TCPTransport> s_transport;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_transport_t erpc_transport_tcp_init(const char *host, uint16_t port, bool isServer)
{
    s_transport.construct(host, port, isServer);
    if (kErpcStatus_Success == s_transport->open())
    {
        return reinterpret_cast<erpc_transport_t>(s_transport.get());
    }
    return NULL;
}

void erpc_transport_tcp_close(void)
{
    s_transport.get()->close(true);
}
