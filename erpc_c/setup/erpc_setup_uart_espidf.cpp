/*
 * Copyright (c) 2020 Sierra Wireless
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_manually_constructed.h"
#include "erpc_transport_setup.h"
#include "erpc_uart_espidf_transport.h"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

static ManuallyConstructed<UartTransport> s_transport;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_transport_t erpc_transport_uart_espidf_init(void *uart, int uart_num, int rx_gpio, int tx_gpio, int rts_gpio, int cts_gpio, int timeout)
{
    s_transport.construct((uart_config_t*) uart, (uart_port_t) uart_num, (gpio_num_t) rx_gpio, (gpio_num_t) tx_gpio, 
                          (gpio_num_t) rts_gpio, (gpio_num_t) cts_gpio);
    if (s_transport->init(timeout) == kErpcStatus_Success)
    {
        return reinterpret_cast<erpc_transport_t>(s_transport.get());
    }
    return NULL;
}
