/*
 * The Clear BSD License
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 * that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
