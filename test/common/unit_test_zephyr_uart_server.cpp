/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_mbf_setup.h"
#include "erpc_server_setup.h"
#include "erpc_simple_server.hpp"
#include "erpc_transport_setup.h"
#include "erpc_uart_zephyr_transport.hpp"

#include "c_test_unit_test_common_server.h"
#include "myAlloc.hpp"
#include "unit_test_wrapped.h"

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////
#define UART_DEVICE_NODE DT_CHOSEN(zephyr_console)

static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

int ::MyAlloc::allocated_ = 0;
erpc_service_t service_common = NULL;
erpc_server_t server;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

int main(void)
{
    if (!device_is_ready(uart_dev))
    {
        printk("UART device not found!");
        return -1;
    }

    erpc_transport_t transport;
    erpc_mbf_t message_buffer_factory;

    transport = erpc_transport_zephyr_uart_init((void *)uart_dev);
    message_buffer_factory = erpc_mbf_dynamic_init();

    /* Init server */
    server = erpc_server_init(transport, message_buffer_factory);

    /* Add test services. This function call erpc_add_service_to_server for all necessary services. */
    add_services_to_server(server);

    /* Add common service */
    add_common_service(server);

    /* Add run server */
    erpc_server_run(server);

    /* Deinit server */
    erpc_server_deinit(server);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Server helper functions
////////////////////////////////////////////////////////////////////////////////

void add_common_service(erpc_server_t server)
{
    service_common = create_Common_service();
    erpc_add_service_to_server(server, service_common);
}

void remove_common_services_from_server(erpc_server_t server, erpc_service_t service)
{
    erpc_remove_service_from_server(server, service);
    destroy_Common_service(service);
}

////////////////////////////////////////////////////////////////////////////////
// Common service implementations here
////////////////////////////////////////////////////////////////////////////////

void quit()
{
    /* removing common services from the server */
    remove_common_services_from_server(server, service_common);

    /* removing individual test services from the server */
    remove_services_from_server(server);

    erpc_server_stop(server);
}

int32_t getServerAllocated()
{
    int result = ::MyAlloc::allocated();
    ::MyAlloc::allocated(0);
    return result;
}
