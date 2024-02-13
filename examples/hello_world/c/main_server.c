/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "c_hello_world_server.h"
#include "config.h"

#include "erpc_server_setup.h"
#include "erpc_transport_setup.h"
#include "erpc_mbf_setup.h"

#include "erpc_error_handler.h"

#include <stdio.h>

/* eRPC call definition */
void printText(const char *text)
{
    printf("%s", text);
}

int main()
{
    erpc_transport_t transport;
    erpc_mbf_t message_buffer_factory;
    erpc_server_t server;
    erpc_service_t service = create_TextService_service();

    /* Init eRPC server infrastructure */
    transport = erpc_transport_tcp_init(ERPC_HOSTNAME, ERPC_PORT, true);
    message_buffer_factory = erpc_mbf_dynamic_init();
    server = erpc_server_init(transport, message_buffer_factory);

    /* Add custom service implementation to the server */
    erpc_add_service_to_server(server, service);

    /* Poll for request */
    erpc_status_t err = erpc_server_poll(server);

    /* Handle error status */
    if (err != kErpcStatus_Success)
    {
        /* Print error description */
        erpc_error_handler(err, 0);
    }

    /* Deinit objects */
    destroy_TextService_service(service);
    erpc_server_deinit(server);
    erpc_mbf_dynamic_deinit(message_buffer_factory);
    erpc_transport_tcp_close(transport);
    erpc_transport_tcp_deinit(transport);
}
