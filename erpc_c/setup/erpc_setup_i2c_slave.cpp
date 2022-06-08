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

static ManuallyConstructed<I2cSlaveTransport> s_transport;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_transport_t erpc_transport_i2c_slave_init(void *baseAddr, uint32_t baudRate, uint32_t srcClock_Hz)
{
    s_transport.construct((I2C_Type *)baseAddr, baudRate, srcClock_Hz);
    (void)s_transport->init();
    return reinterpret_cast<erpc_transport_t>(s_transport.get());
}
