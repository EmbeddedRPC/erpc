/*
 * Copyright 2023 NXP
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_manually_constructed.hpp"
#include "erpc_transport_setup.h"
#include "erpc_mbox_zephyr_transport.hpp"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

ERPC_MANUALLY_CONSTRUCTED(MBOXTransport, s_transport);

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_transport_t erpc_transport_zephyr_mbox_init(void *dev, void *tx_channel, void *rx_channel)
{
    erpc_transport_t transport;
    MBOXTransport *mboxTransport;

#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    if (s_transport.isUsed())
    {
        mboxTransport = NULL;
    }
    else
    {
        s_transport.construct((struct device *)dev, (struct mbox_channel *)tx_channel, (struct mbox_channel *)rx_channel);
        mboxTransport = s_transport.get();
    }

    
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
        mboxTransport = new MBOXTransport((struct device *)dev, (struct mbox_channel *)tx_channel, (struct mbox_channel *)rx_channel);
#else
#error "Unknown eRPC allocation policy!"
#endif

    transport = reinterpret_cast<erpc_transport_t>(mboxTransport);

    if (mboxTransport != NULL)
    {
        if (mboxTransport->init() != kErpcStatus_Success)
        {
            erpc_transport_zephyr_mbox_deinit(transport);
            transport = NULL;
        }
    }

    return transport;
}

void erpc_transport_zephyr_mbox_deinit(erpc_transport_t transport)
{
#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    (void)transport;
    s_transport.destroy();
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    erpc_assert(transport != NULL);

    MBOXTransport *mboxTransport = reinterpret_cast<MBOXTransport *>(transport);

    delete mboxTransport;
#endif
}
