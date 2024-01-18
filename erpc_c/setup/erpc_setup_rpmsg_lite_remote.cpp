/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_manually_constructed.hpp"
#include "erpc_rpmsg_lite_transport.hpp"
#include "erpc_transport_setup.h"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

ERPC_MANUALLY_CONSTRUCTED_STATIC(RPMsgTransport, s_rpmsgTransport);

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_transport_t erpc_transport_rpmsg_lite_remote_init(uint32_t src_addr, uint32_t dst_addr, void *start_address,
                                                       uint32_t rpmsg_link_id, rpmsg_ready_cb ready,
                                                       char *nameservice_name)
{
    erpc_transport_t transport;
    RPMsgTransport *rpmsgTransport;

#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    if (s_rpmsgTransport.isUsed())
    {
        rpmsgTransport = NULL;
    }
    else
    {
        s_rpmsgTransport.construct();
        rpmsgTransport = s_rpmsgTransport.get();
    }
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    rpmsgTransport = new RPMsgTransport();
#else
#error "Unknown eRPC allocation policy!"
#endif

    transport = reinterpret_cast<erpc_transport_t>(rpmsgTransport);

    if (rpmsgTransport != NULL)
    {
        if (rpmsgTransport->init(src_addr, dst_addr, start_address, rpmsg_link_id, ready, nameservice_name) !=
            kErpcStatus_Success)
        {
            erpc_transport_rpmsg_lite_remote_deinit(transport);
            transport = NULL;
        }
    }

    return transport;
}

void erpc_transport_rpmsg_lite_remote_deinit(erpc_transport_t transport)
{
#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    (void)transport;
    s_rpmsgTransport.destroy();
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    erpc_assert(transport != NULL);

    RPMsgTransport *rpmsgTransport = reinterpret_cast<RPMsgTransport *>(transport);

    delete rpmsgTransport;
#endif
}
