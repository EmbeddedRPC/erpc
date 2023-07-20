/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_server_setup.h"

#include "c_test_server.h"
#include "test_server.hpp"
#include "c_test_unit_test_common_server.h"
#include "unit_test.h"
#include "unit_test_wrapped.h"

#include <stdlib.h>
#include <string.h>

using namespace erpc;
using namespace erpcshim;

TypedefService_service *svc;

////////////////////////////////////////////////////////////////////////////////
// Implementation of function code
////////////////////////////////////////////////////////////////////////////////

/* typedef unit tests */
int32type sendReceiveInt(int32type a)
{
    return a * 2 + 1;
}

Colors sendReceiveEnum(Colors a)
{
    return (Colors)((int32_t)a + 1);
}

B *sendReceiveStruct(const B *a)
{
    B *b = (B *)erpc_malloc(sizeof(B));
    b->m = 2 * a->m;
    b->n = 2 + a->n;
    return b;
}

ListType *sendReceiveListType(const ListType *received_list)
{
    ListType *send_list = (ListType *)erpc_malloc(sizeof(ListType));
    send_list->elementsCount = received_list->elementsCount;
    send_list->elements = (int32_t *)erpc_malloc(send_list->elementsCount * sizeof(int32_t));
    int32_t *list_p = send_list->elements;

    for (uint32_t i = 0; i < send_list->elementsCount; ++i)
    {
        *list_p = 2 * (int32_t)i;
        ++list_p;
    }
    return send_list;
}

newString sendReceiveString(newString hello)
{
    char *b = (char *)" World!";
    char *result = (char *)erpc_malloc((strlen(hello) + strlen(b) + 1) * sizeof(char));
    strcpy(result, hello);
    strcat(result, b);
    erpc_free(hello);
    return result;
}

ListType2 *sendReceive2ListType(const ListType2 *received_list)
{
    ListType2 *send_list;
    send_list = (ListType2 *)erpc_malloc(sizeof(ListType2));
    ListType *list_0_r = received_list->elements;
    send_list->elementsCount = received_list->elementsCount;
    send_list->elements = (ListType *)erpc_malloc(send_list->elementsCount * sizeof(ListType));
    ListType *list_0_s = send_list->elements;
    for (uint32_t i = 0; i < send_list->elementsCount; ++i)
    {
        int32_t *list_s, *list_r;
        list_r = list_0_r->elements;
        list_0_s->elementsCount = list_0_r->elementsCount;
        list_0_s->elements = (int32_t *)erpc_malloc(list_0_s->elementsCount * sizeof(int32_t));
        list_s = list_0_s->elements;
        for (uint32_t j = 0; j < list_0_s->elementsCount; ++j)
        {
            *list_s = 2 * (*list_r);
            ++list_r;
            ++list_s;
        }
        ++list_0_r;
        ++list_0_s;
    }

    return send_list;
}

/*int32_t * sendTypeArray(ArrayType arrays){
    uint32_t array_count = 8;
    Log::info("Received these numbers ");
    for(uint32_t i = 0; i < array_count; ++i)
    {
        Log::info("%d ", arrays[i]);
    }
    Log::info("\n ");
    return arrays;
}*/

/*int32_t *** sendMultiListArray(uint32_t &sendMultiListArray_1_count, uint32_t &sendMultiListArray_2_count,
MultiListArray arrayNumbers, uint32_t arrayNumbers_1_count, uint32_t arrayNumbers_2_count)
{
    uint32_t array_count = 2;
    Log::info("Received these numbers ");
    for(uint32_t i = 0; i < arrayNumbers_1_count; ++i)
    {
        Log::info("%d:\n", i);
        for(uint32_t j = 0; j < array_count; ++j)
        {
            Log::info("    %d:\n", j);
            for(uint32_t k = 0; k < arrayNumbers_2_count; ++k)
            {
                Log::info("        %d:%d\n", k, arrayNumbers[i][j][k]);
            }
        }
    }
    sendMultiListArray_1_count = arrayNumbers_1_count;
    sendMultiListArray_2_count = arrayNumbers_2_count;
    Log::info("\n ");
    return (int32_t ***)arrayNumbers;
}*/
/* end typedef unit tests */

class TypedefService_server: public TypedefService_interface
{
    public:

        int32type sendReceiveInt(int32type a)
        {
            int32type result;
            result = ::sendReceiveInt(a);

            return result;
        }

        Colors sendReceiveEnum(Colors a)
        {
            Colors result;
            result = ::sendReceiveEnum(a);

            return result;
        }

        B * sendReceiveStruct(const B * a)
        {
            B * result = NULL;
            result = ::sendReceiveStruct(a);

            return result;
        }

        ListType * sendReceiveListType(const ListType * listNumbers)
        {
            ListType * result = NULL;
            result = ::sendReceiveListType(listNumbers);

            return result;
        }

        ListType2 * sendReceive2ListType(const ListType2 * listNumbers)
        {
            ListType2 * result = NULL;
            result = ::sendReceive2ListType(listNumbers);

            return result;
        }

        newString sendReceiveString(newString hello)
        {
            newString result = NULL;
            result = ::sendReceiveString(hello);

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
    svc = new TypedefService_service(new TypedefService_server());

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
    service_test = create_TypedefService_service();
    erpc_add_service_to_server(server, service_test);
}

void remove_services_from_server(erpc_server_t server)
{
    erpc_remove_service_from_server(server, service_test);
    destroy_TypedefService_service(service_test);
}

#ifdef __cplusplus
}
#endif
