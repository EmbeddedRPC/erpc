/*
 * Copyright 2023 NXP
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_manually_constructed.hpp"
#include "erpc_transport_setup.h"
#include "erpc_uart_zephyr_transport.hpp"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

ERPC_MANUALLY_CONSTRUCTED(UartTransport, s_transport);

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_transport_t erpc_transport_zephyr_uart_init(void *dev)
{
    erpc_transport_t transport;

    s_transport.construct((struct device *)dev);
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
