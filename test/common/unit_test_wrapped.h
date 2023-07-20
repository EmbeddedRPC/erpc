/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _EMBEDDED_RPC__UNIT_TEST_WRAPPED_H_
#define _EMBEDDED_RPC__UNIT_TEST_WRAPPED_H_

#include "erpc_client_manager.h"
#include "erpc_server_setup.h"

////////////////////////////////////////////////////////////////////////////////
// Function Prototypes
////////////////////////////////////////////////////////////////////////////////

typedef void *erpc_service_t;
#ifdef __cplusplus
extern "C" {
#endif
void initInterfaces(erpc_client_t client);
void initInterfaces_common(erpc_client_t client);

void add_services_to_server(erpc_server_t server);
void remove_services_from_server(erpc_server_t server);
void remove_common_services_from_server(erpc_server_t server, erpc_service_t service);
void add_common_service(erpc_server_t server);
#ifdef __cplusplus
}
#endif

#endif // _EMBEDDED_RPC__UNIT_TEST_WRAPPED_H_
