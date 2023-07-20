/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_server_setup.h"

#include "c_test_server.h"
#include "c_test_unit_test_common_server.h"
#include "test_server.hpp"
#include "unit_test.h"
#include "unit_test_wrapped.h"

#include <stdlib.h>

using namespace erpc;
using namespace erpcShim;

EnumsService_service *svc;

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

enumErrorCode test_enumErrorCode_allDirection(enumErrorCode a, enumErrorCode b, enumErrorCode *c, enumErrorCode *e)
{
    *c = a;
    *e = b;
    return a;
}

class EnumsService_server : public EnumsService_interface
{
public:
    void test_enumColor_in(enumColor a) { ::test_enumColor_in(a); }

    void test_enumColor_in2(enumColor b) { ::test_enumColor_in2(b); }

    void test_enumColor_out(enumColor *c) { ::test_enumColor_out(c); }

    void test_enumColor_inout(enumColor *e) { ::test_enumColor_inout(e); }

    enumColor test_enumColor_return(void)
    {
        enumColor result;
        result = ::test_enumColor_return();

        return result;
    }

    enumColor test_enumColor_allDirection(enumColor a, enumColor b, enumColor *c, enumColor *e)
    {
        enumColor result;
        result = ::test_enumColor_allDirection(a, b, c, e);

        return result;
    }

    enumColor2 test_enumColor2_allDirection(enumColor2 a, enumColor2 b, enumColor2 *c, enumColor2 *e)
    {
        enumColor2 result;
        result = ::test_enumColor2_allDirection(a, b, c, e);

        return result;
    }

    enumErrorCode test_enumErrorCode_allDirection(enumErrorCode a, enumErrorCode b, enumErrorCode *c, enumErrorCode *e)
    {
        enumErrorCode result;
        result = ::test_enumErrorCode_allDirection(a, b, c, e);

        return result;
    }
};

////////////////////////////////////////////////////////////////////////////////
// Add service to server code
////////////////////////////////////////////////////////////////////////////////

void add_services(erpc::SimpleServer *server)
{
    // define services to add on heap
    // allocate on heap so service doesn't go out of scope at end of method
    svc = new EnumsService_service(new EnumsService_server());

    // add services
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
    service_test = create_EnumsService_service();
    erpc_add_service_to_server(server, service_test);
}

void remove_services_from_server(erpc_server_t server)
{
    erpc_remove_service_from_server(server, service_test);
    destroy_EnumsService_service(service_test);
}
#ifdef __cplusplus
}
#endif
