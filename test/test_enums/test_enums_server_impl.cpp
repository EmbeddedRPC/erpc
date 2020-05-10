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

struct saveVariables
{
    enumColor enumColorA;
    enumColor enumColorB;
};

struct saveVariables saveVariables1;

void test_enumColor_in(enumColor a)
{
    saveVariables1.enumColorA = a;
}

void test_enumColor_in2(enumColor b)
{
    saveVariables1.enumColorB = b;
}

void test_enumColor_out(enumColor *c)
{
    *c = saveVariables1.enumColorA;
}

void test_enumColor_inout(enumColor *e)
{
    *e = (enumColor)((int32_t)*e - 9);
}

enumColor test_enumColor_return()
{
    return (enumColor)((int32_t)saveVariables1.enumColorA + 9);
}

enumColor test_enumColor_allDirection(enumColor a, enumColor b, enumColor *c, enumColor *e)
{
    *c = a;
    *e = b;
    return a;
}

enumColor2 test_enumColor2_allDirection(enumColor2 a, enumColor2 b, enumColor2 *c, enumColor2 *e)
{
    *c = a;
    *e = b;
    return a;
}

////////////////////////////////////////////////////////////////////////////////
// Add service to server code
////////////////////////////////////////////////////////////////////////////////

void add_services(erpc::SimpleServer *server)
{
    // define services to add on heap
    // allocate on heap so service doesn't go out of scope at end of method
    // NOTE: possible memory leak? not ever deleting
    EnumsService_service *svc = new EnumsService_service();

    // add services
    server->addService(svc);
}

#ifdef __cplusplus
extern "C" {
#endif
void add_services_to_server()
{
    erpc_add_service_to_server(create_EnumsService_service());
}
#ifdef __cplusplus
}
#endif
