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
#include "test_lists_server.h"
#include "unit_test.h"
#include "unit_test_wrapped.h"
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
// Implementation of function code
////////////////////////////////////////////////////////////////////////////////

list_0_t *sendReceivedInt32(const list_0_t *received_list)
{
    list_0_t *send_list = (list_0_t *)erpc_malloc(sizeof(list_0_t));
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

list_1_t *sendReceived2Int32(const list_1_t *received_list)
{
    list_1_t *send_list;
    send_list = (list_1_t *)erpc_malloc(sizeof(list_1_t));
    list_0_t *list_0_t_r = received_list->elements;
    send_list->elementsCount = received_list->elementsCount;
    send_list->elements = (list_0_t *)erpc_malloc(send_list->elementsCount * sizeof(list_0_t));
    list_0_t *list_0_t_s = send_list->elements;
    for (uint32_t i = 0; i < send_list->elementsCount; ++i)
    {
        int32_t *list_s, *list_r;
        list_r = list_0_t_r->elements;
        list_0_t_s->elementsCount = list_0_t_r->elementsCount;
        list_0_t_s->elements = (int32_t *)erpc_malloc(list_0_t_s->elementsCount * sizeof(int32_t));
        list_s = list_0_t_s->elements;
        for (uint32_t j = 0; j < list_0_t_s->elementsCount; ++j)
        {
            *list_s = 2 * (*list_r);
            ++list_r;
            ++list_s;
        }
        ++list_0_t_r;
        ++list_0_t_s;
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

list_2_t *sendReceivedEnum(const list_2_t *received_list)
{
    list_2_t *send_list = (list_2_t *)erpc_malloc(sizeof(list_2_t));
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

list_3_t *sendReceived2Enum(const list_3_t *received_list)
{
    list_3_t *send_list;
    send_list = (list_3_t *)erpc_malloc(sizeof(list_3_t));
    list_2_t *list_2_t_r = received_list->elements;
    send_list->elementsCount = received_list->elementsCount;
    send_list->elements = (list_2_t *)erpc_malloc(send_list->elementsCount * sizeof(list_2_t));
    list_2_t *list_2_t_s = send_list->elements;
    for (uint32_t i = 0; i < send_list->elementsCount; ++i)
    {
        enumColor *list_s, *list_r;
        list_r = list_2_t_r->elements;
        list_2_t_s->elementsCount = list_2_t_r->elementsCount;
        list_2_t_s->elements = (enumColor *)erpc_malloc(list_2_t_s->elementsCount * sizeof(enumColor));
        list_s = list_2_t_s->elements;
        for (uint32_t j = 0; j < list_2_t_s->elementsCount; ++j)
        {
            *list_s = *list_r;
            ++list_r;
            ++list_s;
        }
        ++list_2_t_r;
        ++list_2_t_s;
    }

    return send_list;
}

list_4_t *sendReceivedStruct(const list_4_t *received_list)
{
    list_4_t *send_list = (list_4_t *)erpc_malloc(sizeof(list_4_t));
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

list_5_t *sendReceived2Struct(const list_5_t *received_list)
{
    list_5_t *send_list;
    send_list = (list_5_t *)erpc_malloc(sizeof(list_5_t));
    list_4_t *list_0_t_r = received_list->elements;
    send_list->elementsCount = received_list->elementsCount;
    send_list->elements = (list_4_t *)erpc_malloc(send_list->elementsCount * sizeof(list_4_t));
    list_4_t *list_0_t_s = send_list->elements;
    for (uint32_t i = 0; i < send_list->elementsCount; ++i)
    {
        C *list_s, *list_r;
        list_r = list_0_t_r->elements;
        list_0_t_s->elementsCount = list_0_t_r->elementsCount;
        list_0_t_s->elements = (C *)erpc_malloc(list_0_t_s->elementsCount * sizeof(C));
        list_s = list_0_t_s->elements;
        for (uint32_t j = 0; j < list_0_t_s->elementsCount; ++j)
        {
            list_s->m = 2 * list_r->m;
            list_s->n = 2 * list_r->n;
            ++list_r;
            ++list_s;
        }
        erpc_free(list_0_t_r->elements);
        ++list_0_t_r;
        ++list_0_t_s;
    }
    erpc_free(received_list->elements);
    erpc_free((void *)received_list);

    return send_list;
}

list_6_t *sendReceivedString(const list_6_t *received_list)
{
    list_6_t *send_list = (list_6_t *)erpc_malloc(sizeof(list_6_t));
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

list_7_t *sendReceived2String(const list_7_t *received_list)
{
    list_7_t *send_list;
    char *stringValue = (char *)" World!";
    send_list = (list_7_t *)erpc_malloc(sizeof(list_7_t));
    list_6_t *list_0_t_r = received_list->elements;
    send_list->elementsCount = received_list->elementsCount;
    send_list->elements = (list_6_t *)erpc_malloc(send_list->elementsCount * sizeof(list_6_t));
    list_6_t *list_0_t_s = send_list->elements;
    for (uint32_t i = 0; i < send_list->elementsCount; ++i)
    {
        char **list_s, **list_r;
        list_r = list_0_t_r->elements;
        list_0_t_s->elementsCount = list_0_t_r->elementsCount;
        list_0_t_s->elements = (char **)erpc_malloc(list_0_t_s->elementsCount * sizeof(char *));
        list_s = list_0_t_s->elements;
        for (uint32_t j = 0; j < list_0_t_s->elementsCount; ++j)
        {
            *list_s = (char *)erpc_malloc((strlen(*list_r) + strlen(stringValue) + 1) * sizeof(char));
            strcpy(*list_s, *list_r);
            strcat(*list_s, stringValue);
            ++list_r;
            ++list_s;
        }
        ++list_0_t_r;
        ++list_0_t_s;
    }

    return send_list;
}

void test_list_allDirection(const list_8_t *a, const list_8_t *b, list_8_t *e)
{
    uint32_t *s_list_e = (e)->elements;
    for (uint32_t i = 0; i < (a)->elementsCount; ++i)
    {
        *s_list_e = 2 * (*s_list_e);
        ++s_list_e;
    }
}

/*void test_list_allDirection(const list_0_t *a, const list_0_t *b, list_0_t *c, list_0_t **d, list_0_t *e)
{
    (*d)->elements = (int32_t *)erpc_malloc((a)->elementsCount * sizeof(int32_t));
    c->elements = (int32_t *)erpc_malloc((a)->elementsCount * sizeof(int32_t));
    (*d)->elementsCount = (c)->elementsCount = (a)->elementsCount;
    int32_t *s_list_c = (c)->elements;
    int32_t *s_list_d = (*d)->elements;
    int32_t *s_list_e = (e)->elements;
    for (uint32_t i = 0; i < (a)->elementsCount; ++i)
    {
        *s_list_c = (int32_t)i * 4;
        *s_list_d = (int32_t)i * 5;
        *s_list_e = 2 * (*s_list_e);
        ++s_list_c;
        ++s_list_d;
        ++s_list_e;
    }
    free((void *)a->elements);
    free((void *)b->elements);
    free((void *)a);
    free((void *)b);
}*/

int32_t testLengthAnnotation(int32_t *myList, int32_t len)
{
    int32_t *itr = myList;
    for (int i = 0; i < len; ++i)
    {
        if (*itr != (i + 1))
        {
            return 0;
        }
        ++itr;
    }
    return 1;
}

int32_t testLengthAnnotationInStruct(const listStruct *s)
{
    for (int32_t i = 0; i < s->len; ++i)
    {
        if ((i + 1) != s->myList[i])
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

////////////////////////////////////////////////////////////////////////////////
// Add service to server code
////////////////////////////////////////////////////////////////////////////////

void add_services(erpc::SimpleServer *server)
{
    // define services to add on heap
    // allocate on heap so service doesn't go out of scope at end of method
    // NOTE: possible memory leak? not ever deleting
    PointersService_service *svc = new PointersService_service();

    // add services
    server->addService(svc);
}

#ifdef __cplusplus
extern "C" {
#endif
void add_services_to_server()
{
    erpc_add_service_to_server(create_PointersService_service());
}
#ifdef __cplusplus
}
#endif
