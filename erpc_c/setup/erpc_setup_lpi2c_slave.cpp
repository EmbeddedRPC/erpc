/*
 * Copyright 2022-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_lpi2c_slave_transport.hpp"
#include "erpc_manually_constructed.hpp"
#include "erpc_transport_setup.h"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

ERPC_MANUALLY_CONSTRUCTED_STATIC(LPI2cSlaveTransport, s_lpi2cTransport);

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_transport_t erpc_transport_lpi2c_slave_init(void *baseAddr, uint32_t baudRate, uint32_t srcClock_Hz)
{
    LPI2cSlaveTransport *lpi2cTransport;

#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    if (s_lpi2cTransport.isUsed())
    {
        lpi2cTransport = NULL;
    }
    else
    {
        s_lpi2cTransport.construct(reinterpret_cast<LPI2C_Type *>(baseAddr), baudRate, srcClock_Hz);
        lpi2cTransport = s_lpi2cTransport.get();
    }
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    lpi2cTransport = new LPI2cSlaveTransport(reinterpret_cast<LPI2C_Type *>(baseAddr), baudRate, srcClock_Hz);
#else
#error "Unknown eRPC allocation policy!"
#endif

    if (lpi2cTransport != NULL)
    {
        (void)lpi2cTransport->init();
    }

    return reinterpret_cast<erpc_transport_t>(lpi2cTransport);
}
void erpc_transport_lpi2c_slave_deinit(erpc_transport_t transport)
{
#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    (void)transport;
    s_lpi2cTransport.destroy();
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    erpc_assert(transport != NULL);

    LPI2cSlaveTransport *lpi2cTransport = reinterpret_cast<LPI2cSlaveTransport *>(transport);

    delete lpi2cTransport;
#endif
}
