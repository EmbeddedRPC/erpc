/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_server_setup.h"

#include "test_server.h"
#include "test_unit_test_common_server.h"
#include "unit_test.h"
#include "unit_test_wrapped.h"

#include <stdlib.h>

Binary_service *svc;

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

void sendBinary(const binary_t *a) {}

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

void test_binary_allDirectionLength(const uint8_t *a, const binary_t *b, binary_t *d, uint32_t p1) {}

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
     */
    svc = new Binary_service();

    /* Add services
     * Example: server->addService(svc);
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
    delete svc;
}

#ifdef __cplusplus
extern "C" {
#endif
erpc_service_t service_test = NULL;
void add_services_to_server()
{
    service_test = create_Binary_service();
    erpc_add_service_to_server(service_test);
}

void remove_services_from_server()
{
    erpc_remove_service_from_server(service_test);
#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    destroy_Binary_service((erpc_service_t *)service_test);
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    destroy_Binary_service();
#endif
}

void remove_common_services_from_server(erpc_service_t service)
{
    erpc_remove_service_from_server(service);
#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    destroy_Common_service((erpc_service_t *)service);
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    destroy_Common_service();
#endif
}
#ifdef __cplusplus
}
#endif
