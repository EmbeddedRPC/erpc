/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_server_setup.h"

#include "c_test_server.h"
#include "c_test_unit_test_common_server.h"
#include "unit_test.h"
#include "unit_test_wrapped.h"

#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////
// Implementation of function code
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Add service to server code
////////////////////////////////////////////////////////////////////////////////

void add_services(erpc::SimpleServer *server)
{
    /* Define services to add using dynamic memory allocation
     * Exapmle:ArithmeticService_service * svc = new ArithmeticService_service();
     */

    /* Add services
     * Example: server->addService(svc);
     */
}

////////////////////////////////////////////////////////////////////////////////
// Remove service from server code
////////////////////////////////////////////////////////////////////////////////

void remove_services(erpc::SimpleServer *server)
{
    /* Remove services
     * Example: server->removeService (svc);
     */

    /* Delete unused service
     */
}

#ifdef __cplusplus
extern "C" {
#endif
void add_services_to_server(erpc_server_t server) {}
void remove_services_from_server(erpc_server_t server) {}
#ifdef __cplusplus
}
#endif
