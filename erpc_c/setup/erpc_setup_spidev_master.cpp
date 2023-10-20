/*
 * Copyright 2020-2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_manually_constructed.hpp"
#include "erpc_spidev_master_transport.hpp"
#include "erpc_transport_setup.h"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

ERPC_MANUALLY_CONSTRUCTED_STATIC(SpidevMasterTransport, s_spidevTransport);

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_transport_t erpc_transport_spidev_master_init(const char *spidev, uint32_t speed_Hz)
{
    SpidevMasterTransport *spidevTransport;

#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    if (s_spidevTransport.isUsed())
    {
        spidevTransport = NULL;
    }
    else
    {
        s_spidevTransport.construct(spidev, speed_Hz);
        spidevTransport = s_spidevTransport.get();
    }
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    spidevTransport = new SpidevMasterTransport(spidev, speed_Hz);
#else
#error "Unknown eRPC allocation policy!"
#endif

    if (spidevTransport != NULL)
    {
        (void)spidevTransport->init();
    }

    return reinterpret_cast<erpc_transport_t>(spidevTransport);
}

void erpc_transport_spidev_master_deinit(erpc_transport_t transport)
{
#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    (void)transport;
    s_spidevTransport.destroy();
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    erpc_assert(transport != NULL);

    SpidevMasterTransport *spidevTransport = reinterpret_cast<SpidevMasterTransport *>(transport);

    delete spidevTransport;
#endif
}
