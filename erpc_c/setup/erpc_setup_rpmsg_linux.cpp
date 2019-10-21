/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_manually_constructed.h"
#include "erpc_rpmsg_linux_transport.h"
#include "erpc_transport_setup.h"

using namespace erpc;

static ManuallyConstructed<RPMsgLinuxTransport> s_transport;
static ManuallyConstructed<RPMsgEndpoint> s_endpoint;

erpc_transport_t erpc_transport_rpmsg_linux_init(int16_t local_addr, int8_t type, int16_t remote_addr)
{
    if (local_addr == -1)
    {
        local_addr = RPMsgEndpoint::kLocalDefaultAddress;
    }
    if (type == 0)
    {
        type = RPMsgEndpoint::kDatagram;
    }
    else
    {
        type = RPMsgEndpoint::kStream;
    }
    if (remote_addr == -1)
    {
        remote_addr = RPMsgEndpoint::kRemoteDefaultAddress;
    }

    s_endpoint.construct(local_addr, type, remote_addr);
    s_transport.construct(s_endpoint.get(), remote_addr);

    if (s_transport->init() == kErpcStatus_Success)
    {
        return reinterpret_cast<erpc_transport_t>(s_transport.get());
    }
    return NULL;
}

void erpc_transport_rpmsg_linux_deinit(void)
{
    s_endpoint.destroy();
    s_transport.destroy();
}
