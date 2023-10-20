/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_i2c_slave_transport.hpp"
#include "erpc_manually_constructed.hpp"
#include "erpc_transport_setup.h"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

ERPC_MANUALLY_CONSTRUCTED_STATIC(I2cSlaveTransport, s_i2cTransport);

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_transport_t erpc_transport_i2c_slave_init(void *baseAddr, uint32_t baudRate, uint32_t srcClock_Hz)
{
    I2cSlaveTransport *i2cTransport;

#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    if (s_i2cTransport.isUsed())
    {
        i2cTransport = NULL;
    }
    else
    {
        s_i2cTransport.construct(reinterpret_cast<I2C_Type *>(baseAddr), baudRate, srcClock_Hz);
        i2cTransport = s_i2cTransport.get();
    }
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    i2cTransport = new I2cSlaveTransport(reinterpret_cast<I2C_Type *>(baseAddr), baudRate, srcClock_Hz);
#else
#error "Unknown eRPC allocation policy!"
#endif

    if (i2cTransport != NULL)
    {
        (void)i2cTransport->init();
    }

    return reinterpret_cast<erpc_transport_t>(i2cTransport);
}
void erpc_transport_i2c_slave_deinit(erpc_transport_t transport)
{
#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    (void)transport;
    s_i2cTransport.destroy();
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    erpc_assert(transport != NULL);

    I2cSlaveTransport *i2cTransport = reinterpret_cast<I2cSlaveTransport *>(transport);

    delete i2cTransport;
#endif
}
