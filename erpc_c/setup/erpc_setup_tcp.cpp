/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_manually_constructed.h"
#include "erpc_tcp_transport.h"
#include "erpc_transport_setup.h"
#include <iostream>
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
    if (s_transport->open() == kErpcStatus_Success)
    {
        return reinterpret_cast<erpc_transport_t>(s_transport.get());
    }
	
        return NULL;
}

