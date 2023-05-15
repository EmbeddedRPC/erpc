/*
 * Copyright 2020 (c) Sierra Wireless
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_manually_constructed.hpp"
#include "erpc_tcp_transport.hpp"
#include "erpc_transport_setup.h"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

ERPC_MANUALLY_CONSTRUCTED_STATIC(TCPTransport, s_tcpTransport);

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_transport_t erpc_transport_tcp_init(const char *host, uint16_t port, bool isServer)
{
    erpc_transport_t transport;
    TCPTransport *tcpTransport;

#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    if (s_tcpTransport.isUsed())
    {
        tcpTransport = NULL;
    }
    else
    {
        s_tcpTransport.construct(host, port, isServer);
        tcpTransport = s_tcpTransport.get();
    }
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    tcpTransport = new TCPTransport(host, port, isServer);
#else
#error "Unknown eRPC allocation policy!"
#endif

    transport = reinterpret_cast<erpc_transport_t>(tcpTransport);

    if (tcpTransport != NULL)
    {
        if (tcpTransport->open() != kErpcStatus_Success)
        {
            erpc_transport_tcp_deinit(transport);
            transport = NULL;
        }
    }

    return transport;
}

void erpc_transport_tcp_close(erpc_transport_t transport)
{
    erpc_assert(transport != NULL);

    TCPTransport *tcpTransport = reinterpret_cast<TCPTransport *>(transport);

    tcpTransport->close(true);
}

void erpc_transport_tcp_deinit(erpc_transport_t transport)
{
#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    (void)transport;
    s_tcpTransport.destroy();
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    erpc_assert(transport != NULL);

    TCPTransport *tcpTransport = reinterpret_cast<TCPTransport *>(transport);

    delete tcpTransport;
#endif
}
