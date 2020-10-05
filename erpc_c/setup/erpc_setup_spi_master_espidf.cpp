/*
 * Copyright (c) 2020 Sierra Wireless
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_manually_constructed.h"
#include "erpc_transport_setup.h"
#include "erpc_spi_master_espidf_transport.h"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

static ManuallyConstructed<SpiMasterTransport> s_transport;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_transport_t erpc_transport_spi_master_espidf_init(void *spi, int spi_num, int cs_gpio, int ready_gpio, int speed, int timeout)
{
    s_transport.construct((spi_bus_config_t*) spi, (spi_host_device_t) spi_num, (gpio_num_t) cs_gpio, (gpio_num_t) ready_gpio, speed);
    if (s_transport->init(timeout) == kErpcStatus_Success)
    {
        return reinterpret_cast<erpc_transport_t>(s_transport.get());
    }
    return NULL;
}
