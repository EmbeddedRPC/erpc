/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_server_setup.h"
#include "test_core0_server.h"
#include "test_core1.h"
#include "unit_test.h"
#include "unit_test_wrapped.h"
#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////
// Implementation of function code
////////////////////////////////////////////////////////////////////////////////

callback1_t *cb1 = NULL;
callback2_t *cb2 = NULL;

void myFun(const callback1_t pCallback1_in, callback1_t *pCallback1_out)
{
    cb1 = NULL;
    pCallback1_in(1, 2);
    *pCallback1_out = (callback1_t)cb1;
}

void myFun2(const callback2_t pCallback2_in, callback2_t *pCallback2_out)
{
    cb2 = NULL;
    pCallback2_in(1, 2);
    *pCallback2_out = (callback2_t)cb2;
}

void callback1a(int32_t a, int32_t b)
{
    cb1 = (callback1_t *)callback1a;
}

void callback1b(int32_t param1, int32_t param2)
{
    cb1 = (callback1_t *)callback1b;
}

/* will be shim code in real use case */
void callback2(int32_t param1, int32_t param2)
{
    cb2 = (callback2_t *)callback2;
}

int32_t myFun3(const callback3_t callback, int32_t arg1, int32_t arg2)
{
    return callback(arg1, arg2);
}

int32_t my_add(int32_t arg1, int32_t arg2)
{
    return arg1 + arg2;
}

int32_t my_sub(int32_t arg1, int32_t arg2)
{
    return arg1 - arg2;
}

int32_t my_mul(int32_t arg1, int32_t arg2)
{
    return arg1 * arg2;
}

int32_t my_div(int32_t arg1, int32_t arg2)
{
    if(arg2) {
        return arg1 / arg2;
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Add service to server code
////////////////////////////////////////////////////////////////////////////////

void add_services(erpc::SimpleServer *server)
{
    // define services to add on heap
    // allocate on heap so service doesn't go out of scope at end of method
    // NOTE: possible memory leak? not ever deleting
    ClientCore0Services_service *svc = new ClientCore0Services_service();

    // add services
    server->addService(svc);
}

#ifdef __cplusplus
extern "C" {
#endif
void add_services_to_server()
{
    erpc_add_service_to_server(create_ClientCore0Services_service());
}
#ifdef __cplusplus
}
#endif
