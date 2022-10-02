/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * Copyright 2019-2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_manually_constructed.hpp"
#include "erpc_rpmsg_tty_rtos_transport.hpp"
#include "erpc_transport_setup.h"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

ERPC_MANUALLY_CONSTRUCTED(RPMsgTTYRTOSTransport, s_transport);

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_transport_t erpc_transport_rpmsg_lite_tty_rtos_remote_init(uint32_t src_addr, uint32_t dst_addr,
                                                                void *start_address, uint32_t rpmsg_link_id,
                                                                rpmsg_ready_cb ready, char *nameservice_name)
{
    erpc_transport_t transport;

    s_transport.construct();
    if (s_transport->init(src_addr, dst_addr, start_address, rpmsg_link_id, ready, nameservice_name) ==
        kErpcStatus_Success)
    {
        transport = reinterpret_cast<erpc_transport_t>(s_transport.get());
    }
    else
    {
        transport = NULL;
    }

    return transport;
}

void erpc_transport_rpmsg_lite_tty_rtos_deinit(void)
{
    s_transport.destroy();
}
