/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_manually_constructed.hpp"
#include "erpc_transport_setup.h"
#include "erpc_uart_cmsis_transport.hpp"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

ERPC_MANUALLY_CONSTRUCTED_STATIC(UartTransport, s_uartTransport);

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_transport_t erpc_transport_cmsis_uart_init(void *uartDrv)
{
    erpc_transport_t transport;
    UartTransport *uartTransport;

#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    if (s_uartTransport.isUsed())
    {
        uartTransport = NULL;
    }
    else
    {
        s_uartTransport.construct(reinterpret_cast<ARM_DRIVER_USART *>(uartDrv));
        uartTransport = s_uartTransport.get();
    }
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    uartTransport = new UartTransport(reinterpret_cast<ARM_DRIVER_USART *>(uartDrv));
#else
#error "Unknown eRPC allocation policy!"
#endif

    transport = reinterpret_cast<erpc_transport_t>(uartTransport);

    if (uartTransport != NULL)
    {
        if (uartTransport->init() != kErpcStatus_Success)
        {
            erpc_transport_cmsis_uart_deinit(transport);
            transport = NULL;
        }
    }

    return transport;
}

void erpc_transport_cmsis_uart_deinit(erpc_transport_t transport)
{
#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    (void)transport;
    s_uartTransport.destroy();
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    erpc_assert(transport != NULL);

    UartTransport *uartTransport = reinterpret_cast<UartTransport *>(transport);

    delete uartTransport;
#endif
}
