/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_manually_constructed.h"
#include "erpc_serial_transport.h"
#include "erpc_transport_setup.h"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

ERPC_MANUALLY_CONSTRUCTED(SerialTransport, s_transport);

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////
#if defined(STM32F446xx)
erpc_transport_t erpc_transport_serial_init(USART_TypeDef *portName, long baudRate)
#else
erpc_transport_t erpc_transport_serial_init(const char *portName, long baudRate)
#endif
{
    erpc_transport_t transport;
    const uint8_t vtime = 0;
    const uint8_t vmin = 1;

    s_transport.construct(portName, baudRate);
    if (s_transport->init(vtime, vmin) == kErpcStatus_Success)
    {
        transport = reinterpret_cast<erpc_transport_t>(s_transport.get());
    }
    else
    {
        transport = NULL;
    }

    return transport;
}
