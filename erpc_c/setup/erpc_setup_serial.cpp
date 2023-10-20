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
#include "erpc_serial_transport.hpp"
#include "erpc_transport_setup.h"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

ERPC_MANUALLY_CONSTRUCTED_STATIC(SerialTransport, s_serialTransport);

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_transport_t erpc_transport_serial_init(const char *portName, long baudRate)
{
    erpc_transport_t transport;
    SerialTransport *serialTransport;
    const uint8_t vtime = 0;
    const uint8_t vmin = 1;

#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    if (s_serialTransport.isUsed())
    {
        serialTransport = NULL;
    }
    else
    {
        s_serialTransport.construct(portName, baudRate);
        serialTransport = s_serialTransport.get();
    }
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    serialTransport = new SerialTransport(portName, baudRate);
#else
#error "Unknown eRPC allocation policy!"
#endif

    transport = reinterpret_cast<erpc_transport_t>(serialTransport);

    if (serialTransport != NULL)
    {
        if (serialTransport->init(vtime, vmin) != kErpcStatus_Success)
        {
            erpc_transport_serial_deinit(transport);
            transport = NULL;
        }
    }

    return transport;
}

void erpc_transport_serial_deinit(erpc_transport_t transport)
{
#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    (void)transport;
    s_serialTransport.destroy();
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    erpc_assert(transport != NULL);

    SerialTransport *serialTransport = reinterpret_cast<SerialTransport *>(transport);

    delete serialTransport;
#endif
}
