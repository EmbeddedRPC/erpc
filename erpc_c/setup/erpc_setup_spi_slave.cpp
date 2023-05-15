/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_manually_constructed.hpp"
#include "erpc_spi_slave_transport.hpp"
#include "erpc_transport_setup.h"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

ERPC_MANUALLY_CONSTRUCTED_STATIC(SpiSlaveTransport, s_spiTransport);

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_transport_t erpc_transport_spi_slave_init(void *baseAddr, uint32_t baudRate, uint32_t srcClock_Hz)
{
    SpiSlaveTransport *spiTransport;

#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    if (s_spiTransport.isUsed())
    {
        spiTransport = NULL;
    }
    else
    {
        s_spiTransport.construct(reinterpret_cast<SPI_Type *>(baseAddr), baudRate, srcClock_Hz);
        spiTransport = s_spiTransport.get();
    }
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    spiTransport = new SpiSlaveTransport(reinterpret_cast<SPI_Type *>(baseAddr), baudRate, srcClock_Hz);
#else
#error "Unknown eRPC allocation policy!"
#endif

    if (spiTransport != NULL)
    {
        (void)spiTransport->init();
    }

    return reinterpret_cast<erpc_transport_t>(spiTransport);
}

void erpc_transport_spi_slave_deinit(erpc_transport_t transport)
{
#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    (void)transport;
    s_spiTransport.destroy();
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    erpc_assert(transport != NULL);

    SpiSlaveTransport *spiTransport = reinterpret_cast<SpiSlaveTransport *>(transport);

    delete spiTransport;
#endif
}
