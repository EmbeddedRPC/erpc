/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_server_setup.h"
#include "test_server.h"
#include "unit_test.h"
#include "unit_test_wrapped.h"
#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////
// Implementation of function code
////////////////////////////////////////////////////////////////////////////////

int32_t add(int32_t a, int32_t b)
{
    return a + b;
}

void testIfFooStructExist(const fooStruct *a){};

void testIfMyEnumExist(myEnum a){};

myInt testIfMyIntAndConstExist(myInt a)
{
    return a;
}

////////////////////////////////////////////////////////////////////////////////
// Add service to server code
////////////////////////////////////////////////////////////////////////////////

void add_services(erpc::SimpleServer *server)
{
    /* Define services to add using dynamic memory allocation
    * Exapmle:ArithmeticService_service * svc = new ArithmeticService_service();
    */ // NOTE: possible memory leak? not ever deleting
    AnnotateTest_service *svc = new AnnotateTest_service();
    /* Add services
     * Example: server->addService (svc);
     */
    server->addService(svc);
}

#ifdef __cplusplus
extern "C" {
#endif
void add_services_to_server()
{
    erpc_add_service_to_server(create_AnnotateTest_service());
}
#ifdef __cplusplus
}
#endif
