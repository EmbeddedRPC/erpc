/*
 * Copyright 2017 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_manually_constructed.h"
#include "erpc_rpmsg_linux_transport.h"
#include "erpc_transport_setup.h"

using namespace erpc;

ERPC_MANUALLY_CONSTRUCTED(RPMsgLinuxTransport, s_transport);
ERPC_MANUALLY_CONSTRUCTED(RPMsgEndpoint, s_endpoint);

erpc_transport_t erpc_transport_rpmsg_linux_init(int16_t local_addr, int8_t type, int16_t remote_addr)
{
    erpc_transport_t transport;
    int16_t _local_addr;
    int8_t _type;
    int16_t _remote_addr;

    if (local_addr == -1)
    {
        _local_addr = RPMsgEndpoint::kLocalDefaultAddress;
    }
    else
    {
        _local_addr = local_addr;
    }

    if (type == 0)
    {
        _type = RPMsgEndpoint::kDatagram;
    }
    else
    {
        _type = RPMsgEndpoint::kStream;
    }

    if (remote_addr == -1)
    {
        _remote_addr = RPMsgEndpoint::kRemoteDefaultAddress;
    }
    else
    {
        _remote_addr = remote_addr;
    }

    s_endpoint.construct(_local_addr, _type, _remote_addr);
    s_transport.construct(s_endpoint.get(), _remote_addr);

    if (s_transport->init() == kErpcStatus_Success)
    {
        transport = reinterpret_cast<erpc_transport_t>(s_transport.get());
    }
    else
    {
        transport = NULL;
    }

    return transport;
}

void erpc_transport_rpmsg_linux_deinit(void)
{
    s_endpoint.destroy();
    s_transport.destroy();
}
