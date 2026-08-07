/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_transport_setup.h"
#include "erpc_uart_rtthread_transport.hpp"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_transport_t erpc_transport_rtthread_uart_init(const char *devName, uint32_t baudRate)
{
    erpc_transport_t transport;
    RtThreadUartTransport *uartTransport = new RtThreadUartTransport(devName, baudRate);

    transport = reinterpret_cast<erpc_transport_t>(uartTransport);

    if (uartTransport != NULL)
    {
        if (uartTransport->init() != kErpcStatus_Success)
        {
            erpc_transport_rtthread_uart_deinit(transport);
            transport = NULL;
        }
    }

    return transport;
}

void erpc_transport_rtthread_uart_deinit(erpc_transport_t transport)
{
    erpc_assert(transport != NULL);

    RtThreadUartTransport *uartTransport = reinterpret_cast<RtThreadUartTransport *>(transport);

    delete uartTransport;
}
