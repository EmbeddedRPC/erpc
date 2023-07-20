/*
 * Copyright 2017 - 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_server_setup.h"

#include "test_core0_server.hpp"
#include "c_test_core0_server.h"
#include "c_test_core1_server.h"
#include "unit_test.h"
#include "unit_test_wrapped.h"
#include "erpc_utils.hpp"

#include <stdlib.h>

using namespace erpc;

ClientCore0Services_service *svc;

////////////////////////////////////////////////////////////////////////////////
// Implementation of function code
////////////////////////////////////////////////////////////////////////////////

callback1_t *cb1 = NULL;
callback2_t *cb2 = NULL;

int32_t myFun(const callback1_t pCallback1_in, callback1_t *pCallback1_out)
{
    pCallback1_in(1, 2);
    *pCallback1_out = pCallback1_in;
    return 0;
}

void myFun2(const callback2_t pCallback2_in, callback2_t *pCallback2_out)
{
    pCallback2_in(1, 2);
    *pCallback2_out = pCallback2_in;
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
    if (arg2)
    {
        return arg1 / arg2;
    }
    return 0;
}

static const callback1_t _callback1_t[2] = { callback1a, callback1b };
static const callback2_t _callback2_t[1] = { callback2 };
static const callback3_t _callback3_t[4] = { my_add, my_sub, my_mul, my_div };

class ClientCore0Services_server: public ClientCore0Services_interface
{
    public:

        int32_t myFun(const callback1_t pCallback1_in, callback1_t * pCallback1_out)
        {
            uint16_t _fnIndex;
            ::callback1_t _pCallback1_in = NULL;
            ::callback1_t _pCallback1_out = NULL;
            int32_t result;

            if (ClientCore0Services_interface::get_callbackIdx_callback1_t(&pCallback1_in, _fnIndex))
            {
                _pCallback1_in=::_callback1_t[_fnIndex];
            }

            result = ::myFun(_pCallback1_in, &_pCallback1_out);

            if (findIndexOfFunction((arrayOfFunctionPtr_t)::_callback1_t, sizeof(::_callback1_t)/sizeof(::callback1_t), (functionPtr_t)_pCallback1_out, _fnIndex))
            {
                ClientCore0Services_interface::get_callbackAddress_callback1_t(_fnIndex, pCallback1_out);
            }

            return result;
        }

        void myFun2(const ClientCore1Services_interface::callback2_t pCallback2_in, ClientCore1Services_interface::callback2_t * pCallback2_out)
        {
            uint16_t _fnIndex;
            ::callback2_t _pCallback2_in = NULL;
            ::callback2_t _pCallback2_out = NULL;

            if (ClientCore1Services_interface::get_callbackIdx_callback2_t(&pCallback2_in, _fnIndex))
            {
                _pCallback2_in=::_callback2_t[_fnIndex];
            }

            ::myFun2(_pCallback2_in, &_pCallback2_out);

            if (findIndexOfFunction((arrayOfFunctionPtr_t)::_callback2_t, sizeof(::_callback2_t)/sizeof(::callback2_t), (functionPtr_t)_pCallback2_out, _fnIndex))
            {
                ClientCore1Services_interface::get_callbackAddress_callback2_t(_fnIndex, pCallback2_out);
            }
        }

        int32_t myFun3(const callback3_t callback, int32_t arg1, int32_t arg2)
        {
            uint16_t _fnIndex;
            ::callback3_t _callback = NULL;
            int32_t result;

            if (ClientCore0Services_interface::get_callbackIdx_callback3_t(&callback, _fnIndex))
            {
                _callback=::_callback3_t[_fnIndex];
            }

            result = ::myFun3(_callback, arg1, arg2);

            return result;
        }

        int32_t my_add(int32_t arg1, int32_t arg2)
        {
            int32_t result;
            result = ::my_add(arg1, arg2);

            return result;
        }

        int32_t my_sub(int32_t arg1, int32_t arg2)
        {
            int32_t result;
            result = ::my_sub(arg1, arg2);

            return result;
        }

        int32_t my_mul(int32_t arg1, int32_t arg2)
        {
            int32_t result;
            result = ::my_mul(arg1, arg2);

            return result;
        }

        int32_t my_div(int32_t arg1, int32_t arg2)
        {
            int32_t result;
            result = ::my_div(arg1, arg2);

            return result;
        }

        void callback1a(int32_t a, int32_t b)
        {
            ::callback1a(a, b);
        }

        void callback1b(int32_t param1, int32_t param2)
        {
            ::callback1b(param1, param2);
        }
};


////////////////////////////////////////////////////////////////////////////////
// Add service to server code
////////////////////////////////////////////////////////////////////////////////

void add_services(erpc::SimpleServer *server)
{
    // define services to add on heap
    // allocate on heap so service doesn't go out of scope at end of method
    svc = new ClientCore0Services_service(new ClientCore0Services_server());

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
    delete svc;
}

#ifdef __cplusplus
extern "C" {
#endif
erpc_service_t service_test = NULL;
void add_services_to_server(erpc_server_t server)
{
    service_test = create_ClientCore0Services_service();
    erpc_add_service_to_server(server, service_test);
}

void remove_services_from_server(erpc_server_t server)
{
    erpc_remove_service_from_server(server, service_test);
    destroy_ClientCore0Services_service(service_test);
}
#ifdef __cplusplus
}
#endif
