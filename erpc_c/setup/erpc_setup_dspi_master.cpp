/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_dspi_master_transport.h"
#include "erpc_manually_constructed.h"
#include "erpc_transport_setup.h"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

ERPC_MANUALLY_CONSTRUCTED(DspiMasterTransport, s_transport);

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_transport_t erpc_transport_dspi_master_init(void *baseAddr, uint32_t baudRate, uint32_t srcClock_Hz)
{
    s_transport.construct((SPI_Type *)baseAddr, baudRate, srcClock_Hz);
    s_transport->init();
    return reinterpret_cast<erpc_transport_t>(s_transport.get());
}
