/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_lpspi_slave_transport.hpp"
#include "erpc_manually_constructed.hpp"
#include "erpc_transport_setup.h"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

ERPC_MANUALLY_CONSTRUCTED_STATIC(LPSpiSlaveTransport, s_lpspiTransport);

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_transport_t erpc_transport_lpspi_slave_init(void *baseAddr, uint32_t baudRate, uint32_t srcClock_Hz)
{
    LPSpiSlaveTransport *lpspiTransport;

#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    if (s_lpspiTransport.isUsed())
    {
        lpspiTransport = NULL;
    }
    else
    {
        s_lpspiTransport.construct(reinterpret_cast<LPSPI_Type *>(baseAddr), baudRate, srcClock_Hz);
        lpspiTransport = s_lpspiTransport.get();
    }
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    lpspiTransport = new LPSpiSlaveTransport(reinterpret_cast<LPSPI_Type *>(baseAddr), baudRate, srcClock_Hz);
#else
#error "Unknown eRPC allocation policy!"
#endif

    if (lpspiTransport != NULL)
    {
        (void)lpspiTransport->init();
    }

    return reinterpret_cast<erpc_transport_t>(lpspiTransport);
}

void erpc_transport_lpspi_slave_deinit(erpc_transport_t transport)
{
#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    (void)transport;
    s_lpspiTransport.destroy();
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    erpc_assert(transport != NULL);

    LPSpiSlaveTransport *lpspiTransport = reinterpret_cast<LPSpiSlaveTransport *>(transport);

    delete lpspiTransport;
#endif
}
