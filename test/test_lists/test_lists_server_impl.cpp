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

PointersService_service *svc;

////////////////////////////////////////////////////////////////////////////////
// Implementation of function code
////////////////////////////////////////////////////////////////////////////////

list_int32_1_t *sendReceivedInt32(const list_int32_1_t *received_list)
{
    list_int32_1_t *send_list = (list_int32_1_t *)erpc_malloc(sizeof(list_int32_1_t));
    send_list->elementsCount = received_list->elementsCount;
    int32_t *list_s = send_list->elements = (int32_t *)erpc_malloc(send_list->elementsCount * sizeof(int32_t));
    int32_t *list_r = received_list->elements;

    for (uint32_t i = 0; i < send_list->elementsCount; ++i)
    {
        *list_s = 2 * (*list_r);
        ++list_s;
        ++list_r;
    }
    return send_list;
}

list_int32_2_t *sendReceived2Int32(const list_int32_2_t *received_list)
{
    list_int32_2_t *send_list;
    send_list = (list_int32_2_t *)erpc_malloc(sizeof(list_int32_2_t));
    list_int32_1_t *list_int32_1_t_r = received_list->elements;
    send_list->elementsCount = received_list->elementsCount;
    send_list->elements = (list_int32_1_t *)erpc_malloc(send_list->elementsCount * sizeof(list_int32_1_t));
    list_int32_1_t *list_int32_1_t_s = send_list->elements;
    for (uint32_t i = 0; i < send_list->elementsCount; ++i)
    {
        int32_t *list_s, *list_r;
        list_r = list_int32_1_t_r->elements;
        list_int32_1_t_s->elementsCount = list_int32_1_t_r->elementsCount;
        list_int32_1_t_s->elements = (int32_t *)erpc_malloc(list_int32_1_t_s->elementsCount * sizeof(int32_t));
        list_s = list_int32_1_t_s->elements;
        for (uint32_t j = 0; j < list_int32_1_t_s->elementsCount; ++j)
        {
            *list_s = 2 * (*list_r);
            ++list_r;
            ++list_s;
        }
        ++list_int32_1_t_r;
        ++list_int32_1_t_s;
    }

    return send_list;
}

// void receivedInt32Length(int32_t *listNumbers, uint32_t listSize1)
// {
//     int32_t *list_s = listNumbers;
//     int32_t *list_r = listNumbers2;
//
//     for (uint32_t i = 0; i < listSize; ++i)
//     {
//         *list_s = 2 * (*list_r);
//         ++list_s;
//         ++list_r;
//     }
// }
//
// void receivedInt32Length2(int32_t *listNumbers1, uint32_t listSize1)
// {
//     int32_t *list_s = listNumbers;
//     int32_t *list_r = listNumbers2;
//
//     for (uint32_t i = 0; i < listSize; ++i)
//     {
//         *list_s = 2 * (*list_r);
//         ++list_s;
//         ++list_r;
//     }
// }

list_enumColor_1_t *sendReceivedEnum(const list_enumColor_1_t *received_list)
{
    list_enumColor_1_t *send_list = (list_enumColor_1_t *)erpc_malloc(sizeof(list_enumColor_1_t));
    send_list->elementsCount = received_list->elementsCount;
    enumColor *list_s = send_list->elements = (enumColor *)erpc_malloc(send_list->elementsCount * sizeof(enumColor));
    enumColor *list_r = received_list->elements;

    for (uint32_t i = 0; i < send_list->elementsCount; ++i)
    {
        *list_s = *list_r;
        ++list_s;
        ++list_r;
    }

    return send_list;
}

list_enumColor_2_t *sendReceived2Enum(const list_enumColor_2_t *received_list)
{
    list_enumColor_2_t *send_list;
    send_list = (list_enumColor_2_t *)erpc_malloc(sizeof(list_enumColor_2_t));
    list_enumColor_1_t *list_enumColor_1_t_r = received_list->elements;
    send_list->elementsCount = received_list->elementsCount;
    send_list->elements = (list_enumColor_1_t *)erpc_malloc(send_list->elementsCount * sizeof(list_enumColor_1_t));
    list_enumColor_1_t *list_enumColor_1_t_s = send_list->elements;
    for (uint32_t i = 0; i < send_list->elementsCount; ++i)
    {
        enumColor *list_s, *list_r;
        list_r = list_enumColor_1_t_r->elements;
        list_enumColor_1_t_s->elementsCount = list_enumColor_1_t_r->elementsCount;
        list_enumColor_1_t_s->elements =
            (enumColor *)erpc_malloc(list_enumColor_1_t_s->elementsCount * sizeof(enumColor));
        list_s = list_enumColor_1_t_s->elements;
        for (uint32_t j = 0; j < list_enumColor_1_t_s->elementsCount; ++j)
        {
            *list_s = *list_r;
            ++list_r;
            ++list_s;
        }
        ++list_enumColor_1_t_r;
        ++list_enumColor_1_t_s;
    }

    return send_list;
}

list_C_1_t *sendReceivedStruct(const list_C_1_t *received_list)
{
    list_C_1_t *send_list = (list_C_1_t *)erpc_malloc(sizeof(list_C_1_t));
    send_list->elementsCount = received_list->elementsCount;
    C *list_s = send_list->elements = (C *)erpc_malloc(send_list->elementsCount * sizeof(C));
    C *list_r = received_list->elements;

    for (uint32_t i = 0; i < send_list->elementsCount; ++i)
    {
        list_s->m = 2 * list_r->m;
        list_s->n = 2 * list_r->n;
        ++list_s;
        ++list_r;
    }

    return send_list;
}

list_C_2_t *sendReceived2Struct(const list_C_2_t *received_list)
{
    list_C_2_t *send_list;
    send_list = (list_C_2_t *)erpc_malloc(sizeof(list_C_2_t));
    list_C_1_t *list_int32_1_t_r = received_list->elements;
    send_list->elementsCount = received_list->elementsCount;
    send_list->elements = (list_C_1_t *)erpc_malloc(send_list->elementsCount * sizeof(list_C_1_t));
    list_C_1_t *list_int32_1_t_s = send_list->elements;
    for (uint32_t i = 0; i < send_list->elementsCount; ++i)
    {
        C *list_s, *list_r;
        list_r = list_int32_1_t_r->elements;
        list_int32_1_t_s->elementsCount = list_int32_1_t_r->elementsCount;
        list_int32_1_t_s->elements = (C *)erpc_malloc(list_int32_1_t_s->elementsCount * sizeof(C));
        list_s = list_int32_1_t_s->elements;
        for (uint32_t j = 0; j < list_int32_1_t_s->elementsCount; ++j)
        {
            list_s->m = 2 * list_r->m;
            list_s->n = 2 * list_r->n;
            ++list_r;
            ++list_s;
        }
        erpc_free(list_int32_1_t_r->elements);
        ++list_int32_1_t_r;
        ++list_int32_1_t_s;
    }
    erpc_free(received_list->elements);
    erpc_free((void *)received_list);

    return send_list;
}

list_string_1_t *sendReceivedString(const list_string_1_t *received_list)
{
    list_string_1_t *send_list = (list_string_1_t *)erpc_malloc(sizeof(list_string_1_t));
    char *stringValue = (char *)" World!";
    send_list->elementsCount = received_list->elementsCount;
    char **list_s = send_list->elements = (char **)erpc_malloc(send_list->elementsCount * sizeof(char *));
    char **list_r = received_list->elements;

    for (uint32_t i = 0; i < send_list->elementsCount; ++i)
    {
        *list_s = (char *)erpc_malloc((strlen(*list_r) + strlen(stringValue) + 1) * sizeof(char));
        strcpy(*list_s, *list_r);
        strcat(*list_s, stringValue);
        ++list_s;
        ++list_r;
    }

    return send_list;
}

list_string_2_t *sendReceived2String(const list_string_2_t *received_list)
{
    list_string_2_t *send_list;
    char *stringValue = (char *)" World!";
    send_list = (list_string_2_t *)erpc_malloc(sizeof(list_string_2_t));
    list_string_1_t *list_int32_1_t_r = received_list->elements;
    send_list->elementsCount = received_list->elementsCount;
    send_list->elements = (list_string_1_t *)erpc_malloc(send_list->elementsCount * sizeof(list_string_1_t));
    list_string_1_t *list_int32_1_t_s = send_list->elements;
    for (uint32_t i = 0; i < send_list->elementsCount; ++i)
    {
        char **list_s, **list_r;
        list_r = list_int32_1_t_r->elements;
        list_int32_1_t_s->elementsCount = list_int32_1_t_r->elementsCount;
        list_int32_1_t_s->elements = (char **)erpc_malloc(list_int32_1_t_s->elementsCount * sizeof(char *));
        list_s = list_int32_1_t_s->elements;
        for (uint32_t j = 0; j < list_int32_1_t_s->elementsCount; ++j)
        {
            *list_s = (char *)erpc_malloc((strlen(*list_r) + strlen(stringValue) + 1) * sizeof(char));
            strcpy(*list_s, *list_r);
            strcat(*list_s, stringValue);
            ++list_r;
            ++list_s;
        }
        ++list_int32_1_t_r;
        ++list_int32_1_t_s;
    }

    return send_list;
}

void test_list_allDirection(const list_uint32_1_t *a, const list_uint32_1_t *b, list_uint32_1_t *e)
{
    uint32_t *s_list_e = (e)->elements;
    for (uint32_t i = 0; i < (a)->elementsCount; ++i)
    {
        *s_list_e = 2 * (*s_list_e);
        ++s_list_e;
    }
}

int32_t testLengthAnnotation(const int32_t *myList, uint32_t len)
{
    const int32_t *itr = myList;
    for (uint32_t i = 0; i < len; ++i)
    {
        if (*itr != (int32_t)(i + 1))
        {
            return 0;
        }
        ++itr;
    }
    return 1;
}

int32_t testLengthAnnotationInStruct(const listStruct *s)
{
    for (uint32_t i = 0; i < s->len; ++i)
    {
        if ((int32_t)(i + 1) != s->myList[i])
        {
            return 0;
        }
    }
    return 10;
}

listStruct *returnSentStructLengthAnnotation(const listStruct *s)
{
    listStruct *ret = (listStruct *)erpc_malloc(sizeof(listStruct));
    ret->len = s->len;
    ret->myList = (int32_t *)erpc_malloc(ret->len * sizeof(int32_t));
    for (uint32_t i = 0; i < ret->len; ++i)
    {
        ret->myList[i] = s->myList[i];
    }
    return ret;
}

int32_t sendGapAdvertisingData(const gapAdvertisingData_t *ad)
{
    gapAdStructure_t *adStructPtr = ad->aAdStructures;
    for (int i = 0; i < ad->cNumAdStructures; ++i)
    {
        gapAdStructure_t *adData = adStructPtr;
        if (5 != adData->adType)
        {
            return 0;
        }
        for (uint32_t j = 0; j < adData->length; ++j)
        {
            if (((j + 1) * 5) != adData->aData[j])
            {
                return 0;
            }
        }
        adData++;
    }
    return 33;
}

class PointersService_server: public PointersService_interface
{
    public:

        list_int32_1_t * sendReceivedInt32(const list_int32_1_t * listNumbers)
        {
            list_int32_1_t * result = NULL;
            result = ::sendReceivedInt32(listNumbers);

            return result;
        }

        list_int32_2_t * sendReceived2Int32(const list_int32_2_t * listNumbers)
        {
            list_int32_2_t * result = NULL;
            result = ::sendReceived2Int32(listNumbers);

            return result;
        }

        list_enumColor_1_t * sendReceivedEnum(const list_enumColor_1_t * listColors)
        {
            list_enumColor_1_t * result = NULL;
            result = ::sendReceivedEnum(listColors);

            return result;
        }

        list_enumColor_2_t * sendReceived2Enum(const list_enumColor_2_t * listColors)
        {
            list_enumColor_2_t * result = NULL;
            result = ::sendReceived2Enum(listColors);

            return result;
        }

        list_C_1_t * sendReceivedStruct(const list_C_1_t * listColors)
        {
            list_C_1_t * result = NULL;
            result = ::sendReceivedStruct(listColors);

            return result;
        }

        list_C_2_t * sendReceived2Struct(const list_C_2_t * listColors)
        {
            list_C_2_t * result = NULL;
            result = ::sendReceived2Struct(listColors);

            return result;
        }

        list_string_1_t * sendReceivedString(const list_string_1_t * listNumbers)
        {
            list_string_1_t * result = NULL;
            result = ::sendReceivedString(listNumbers);

            return result;
        }

        list_string_2_t * sendReceived2String(const list_string_2_t * listNumbers)
        {
            list_string_2_t * result = NULL;
            result = ::sendReceived2String(listNumbers);

            return result;
        }

        void test_list_allDirection(const list_uint32_1_t * a, const list_uint32_1_t * b, list_uint32_1_t * e)
        {
            ::test_list_allDirection(a, b, e);
        }

        int32_t testLengthAnnotation(const int32_t * myList, uint32_t len)
        {
            int32_t result;
            result = ::testLengthAnnotation(myList, len);

            return result;
        }

        int32_t testLengthAnnotationInStruct(const listStruct * s)
        {
            int32_t result;
            result = ::testLengthAnnotationInStruct(s);

            return result;
        }

        listStruct * returnSentStructLengthAnnotation(const listStruct * s)
        {
            listStruct * result = NULL;
            result = ::returnSentStructLengthAnnotation(s);

            return result;
        }

        int32_t sendGapAdvertisingData(const gapAdvertisingData_t * ad)
        {
            int32_t result;
            result = ::sendGapAdvertisingData(ad);

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
    svc = new PointersService_service(new PointersService_server());

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
    service_test = create_PointersService_service();
    erpc_add_service_to_server(server, service_test);
}

void remove_services_from_server(erpc_server_t server)
{
    erpc_remove_service_from_server(server, service_test);
    destroy_PointersService_service(service_test);
}

#ifdef __cplusplus
}
#endif
