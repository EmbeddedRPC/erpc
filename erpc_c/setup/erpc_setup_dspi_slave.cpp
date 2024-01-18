/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_dspi_slave_transport.hpp"
#include "erpc_manually_constructed.hpp"
#include "erpc_transport_setup.h"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

ERPC_MANUALLY_CONSTRUCTED_STATIC(DspiSlaveTransport, s_dspiTransport);

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_transport_t erpc_transport_dspi_slave_init(void *baseAddr, uint32_t baudRate, uint32_t srcClock_Hz)
{
    DspiSlaveTransport *dspiTransport;

#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    if (s_dspiTransport.isUsed())
    {
        dspiTransport = NULL;
    }
    else
    {
        s_dspiTransport.construct(reinterpret_cast<SPI_Type *>(baseAddr), baudRate, srcClock_Hz);
        dspiTransport = s_dspiTransport.get();
    }
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    dspiTransport = new DspiSlaveTransport(reinterpret_cast<SPI_Type *>(baseAddr), baudRate, srcClock_Hz);
#else
#error "Unknown eRPC allocation policy!"
#endif

    if (dspiTransport != NULL)
    {
        (void)dspiTransport->init();
    }

    return reinterpret_cast<erpc_transport_t>(dspiTransport);
}

void erpc_transport_dspi_slave_deinit(erpc_transport_t transport)
{
#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    (void)transport;
    s_dspiTransport.destroy();
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    erpc_assert(transport != NULL);

    DspiSlaveTransport *dspiTransport = reinterpret_cast<DspiSlaveTransport *>(transport);

    delete dspiTransport;
#endif
}
