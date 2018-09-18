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

void myFun(const callback1_t pCallback1_t, callback1_t *pCallback2_t)
{
    if (pCallback1_t == callback1)
    {
        *pCallback2_t = pCallback1_t;
    }
    else
    {
        *pCallback2_t = callback2;
    }
}

void myFun2(const callback2_t pCallback1_t, callback2_t *pCallback2_t)
{

    if (pCallback1_t == callback3)
    {
        *pCallback2_t = pCallback1_t;
    }
    else
    {
        *pCallback2_t = NULL;
    }
}

void callback1(int32_t a, int32_t b) {}

void callback2(int32_t param1, int32_t param2) {}

/* will be shim code in real use case */
void callback3(int32_t param1, int32_t param2) {}

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
