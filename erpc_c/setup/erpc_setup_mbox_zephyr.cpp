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

    s_transport.construct((struct device *)dev, (struct mbox_channel *)tx_channel, (struct mbox_channel *)rx_channel);
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
