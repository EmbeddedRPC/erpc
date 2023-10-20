/*
 * Copyright 2017 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_manually_constructed.hpp"
#include "erpc_rpmsg_linux_transport.hpp"
#include "erpc_transport_setup.h"

using namespace erpc;

ERPC_MANUALLY_CONSTRUCTED_STATIC(RPMsgLinuxTransport, s_rpmsgTransport);
ERPC_MANUALLY_CONSTRUCTED_STATIC(RPMsgEndpoint, s_endpoint);

erpc_transport_t erpc_transport_rpmsg_linux_init(int16_t local_addr, int8_t type, int16_t remote_addr)
{
    erpc_transport_t transport;
    int16_t _local_addr;
    int8_t _type;
    int16_t _remote_addr;
    RPMsgLinuxTransport *rpmsgTransport;

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

#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    if (s_endpoint.isUsed() || s_rpmsgTransport.isUsed())
    {
        rpmsgTransport = NULL;
    }
    else
    {
        s_endpoint.construct(_local_addr, _type, _remote_addr);
        s_rpmsgTransport.construct(s_endpoint.get(), _remote_addr);
        rpmsgTransport = s_rpmsgTransport.get();
    }
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    RPMsgEndpoint *endpoint = new RPMsgEndpoint(_local_addr, _type, _remote_addr);
    if (endpoint == NULL)
    {
        rpmsgTransport = NULL;
    }
    else
    {
        rpmsgTransport = new RPMsgLinuxTransport(endpoint, _remote_addr);
    }
#else
#error "Unknown eRPC allocation policy!"
#endif

    transport = reinterpret_cast<erpc_transport_t>(rpmsgTransport);

    if (rpmsgTransport != NULL)
    {
        if (rpmsgTransport->init() != kErpcStatus_Success)
        {
            erpc_transport_rpmsg_linux_deinit(transport);
            transport = NULL;
        }
    }

    return transport;
}

void erpc_transport_rpmsg_linux_deinit(erpc_transport_t transport)
{
#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    (void)transport;
    s_endpoint.destroy();
    s_transport.destroy();
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    erpc_assert(transport != NULL);

    RPMsgLinuxTransport *rpmsgTransport = reinterpret_cast<RPMsgLinuxTransport *>(transport);

    delete rpmsgTransport->getRpmsgEndpoint();
    delete rpmsgTransport;
#endif
}
