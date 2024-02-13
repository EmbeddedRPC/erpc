/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "c_hello_world_client.h"
#include "config.h"

#include "erpc_client_setup.h"
#include "erpc_transport_setup.h"
#include "erpc_mbf_setup.h"

#include "erpc_error_handler.h"

int main()
{
    erpc_transport_t transport;
    erpc_mbf_t message_buffer_factory;
    erpc_client_t client_manager;

    /* Init eRPC client infrastructure */
    transport = erpc_transport_tcp_init(ERPC_HOSTNAME, ERPC_PORT, false);
    message_buffer_factory = erpc_mbf_dynamic_init();
    client_manager = erpc_client_init(transport, message_buffer_factory);

    /* Init eRPC client TextService service */
    initTextService_client(client_manager);

    /* Do eRPC call */
    printText("Hello world!");

    /* Deinit objects */
    deinitTextService_client();
    erpc_client_deinit(client_manager);
    erpc_mbf_dynamic_deinit(message_buffer_factory);
    erpc_transport_tcp_close(transport);
    erpc_transport_tcp_deinit(transport);
}
