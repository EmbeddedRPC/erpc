/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
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
#include "test_annotations_server.h"
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
