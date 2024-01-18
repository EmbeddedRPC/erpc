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
#include "test_server.hpp"
#include "test_unit_test_common_server.hpp"
#include "unit_test.h"
#include "unit_test_wrapped.h"

#include <stdlib.h>

using namespace erpc;
using namespace erpcShim;

AnnotateTest_service *svc;

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

class AnnotateTest_server : public AnnotateTest_interface
{
public:
    int32_t add(int32_t a, int32_t b)
    {
        int32_t result;
        result = ::add(a, b);

        return result;
    }

    void testIfFooStructExist(const fooStruct *a) { ::testIfFooStructExist(a); }

    void testIfMyEnumExist(myEnum a) { ::testIfMyEnumExist(a); }

    myInt testIfMyIntAndConstExist(myInt a)
    {
        myInt result;
        result = ::testIfMyIntAndConstExist(a);

        return result;
    }
};

////////////////////////////////////////////////////////////////////////////////
// Add service to server code
////////////////////////////////////////////////////////////////////////////////

void add_services(erpc::SimpleServer *server)
{
    /* Define services to add using dynamic memory allocation
     * Exapmle:ArithmeticService_service * svc = new ArithmeticService_service();
     */
    svc = new AnnotateTest_service(new AnnotateTest_server());
    /* Add services
     * Example: server->addService (svc);
     */
    server->addService(svc);
}

////////////////////////////////////////////////////////////////////////////////
// Remove service from server code
////////////////////////////////////////////////////////////////////////////////

void remove_services(erpc::SimpleServer *server)
{
    /* Remove services
     * Example: server->removeService (svc);
     */
    server->removeService(svc);
    /* Delete unused service
     */
    delete svc->getHandler();
    delete svc;
}

#ifdef __cplusplus
extern "C" {
#endif
erpc_service_t service_test = NULL;
void add_services_to_server(erpc_server_t server)
{
    service_test = create_AnnotateTest_service();
    erpc_add_service_to_server(server, service_test);
}

void remove_services_from_server(erpc_server_t server)
{
    erpc_remove_service_from_server(server, service_test);
    destroy_AnnotateTest_service(service_test);
}
#ifdef __cplusplus
}
#endif
