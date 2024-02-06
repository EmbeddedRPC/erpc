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
    UartTransport *uartTransport;

#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    if (s_transport.isUsed())
    {
        uartTransport = NULL;
    }
    else
    {
        s_transport.construct(reinterpret_cast<struct device *>(dev));
        uartTransport = s_transport.get();
    }
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    uartTransport = new UartTransport(reinterpret_cast<struct device *>(dev));
#else
#error "Unknown eRPC allocation policy!"
#endif

    transport = reinterpret_cast<erpc_transport_t>(uartTransport);

    if (uartTransport != NULL)
    {
        if (uartTransport->init() != kErpcStatus_Success)
        {
            erpc_transport_zephyr_uart_deinit(transport);
            transport = NULL;
        }
    }

    return transport;
}

void erpc_transport_zephyr_uart_deinit(erpc_transport_t transport)
{
#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    (void)transport;
    s_transport.destroy();
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    erpc_assert(transport != NULL);

    UartTransport *uartTransport = reinterpret_cast<UartTransport *>(transport);

    delete uartTransport;
#endif
}