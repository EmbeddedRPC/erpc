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
#include "test_server.h"
#include "unit_test.h"
#include "unit_test_wrapped.h"
#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////
// Implementation of function code
////////////////////////////////////////////////////////////////////////////////

/*binary *sendReceiveBinary(const binary *a)
{
    binary *result = (binary *)erpc_malloc(sizeof(binary));
    result->data = a->data;
    result->dataLength = a->dataLength;
    free((void *)a);
    return result;
}*/

void sendBinary(const binary_t *a)
{
}

/*binary *receiveBinary()
{
    binary *result = (binary *)erpc_malloc(sizeof(binary));
    result->dataLength = 5;
    result->data = (uint8_t *)erpc_malloc(result->dataLength * sizeof(uint8_t));
    for (uint8_t i = 0; i < result->dataLength; ++i)
    {
        result->data[i] = i;
    }
    return result;
}*/

void test_binary_allDirection(const binary_t *a, const binary_t *b, binary_t *e)
{
    for (uint32_t i = 0; i < a->dataLength; ++i)
    {
        e->data[i] = a->data[i] * b->data[i];
    }
}

void test_binary_allDirectionLength(uint8_t *a, const binary_t *b, binary_t *d, uint32_t p1)
{
}

/*void test_binary_allDirection(const binary *a, const binary *b, binary *c, binary **d, binary *e)
{
    c->dataLength = (*d)->dataLength = a->dataLength;
    (*d)->data = (uint8_t *)erpc_malloc((*d)->dataLength * sizeof(uint8_t));
    c->data = (uint8_t *)erpc_malloc(c->dataLength * sizeof(uint8_t));
    for (uint32_t i = 0; i < a->dataLength; ++i)
    {
        c->data[i] = a->data[i];
        (*d)->data[i] = b->data[i];
        e->data[i] = 4 * e->data[i];
    }
    free(a->data);
    free(b->data);
    free((void *)a);
    free((void *)b);
}*/

////////////////////////////////////////////////////////////////////////////////
// Add service to server code
////////////////////////////////////////////////////////////////////////////////

void add_services(erpc::SimpleServer *server)
{
    /* Define services to add using dynamic memory allocation
    * Exapmle:ArithmeticService_service * svc = new ArithmeticService_service();
    */ // NOTE: possible memory leak? not ever deleting
    Binary_service *svc = new Binary_service();

    /* Add services
    * Example: server->addService(svc);
    */
    server->addService(svc);
}

#ifdef __cplusplus
extern "C" {
#endif
void add_services_to_server()
{
    erpc_add_service_to_server(create_Binary_service());
}
#ifdef __cplusplus
}
#endif
