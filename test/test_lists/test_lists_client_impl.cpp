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

#include "gtest.h"
#include "test.h"
#include <string.h>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Unit test Implementation code
////////////////////////////////////////////////////////////////////////////////

TEST(test_list, SendReceivedInt32)
{
    list_0_t *received_list, send_list;
    send_list.elementsCount = 12;
    send_list.elements = (int32_t *)erpc_malloc(send_list.elementsCount * sizeof(int32_t));
    int32_t *list_s = send_list.elements;
    for (uint32_t i = 0; i < send_list.elementsCount; ++i)
    {
        *list_s = (int32_t)i;
        ++list_s;
    }

    received_list = sendReceivedInt32(&send_list);

    list_s = send_list.elements;
    int32_t *list_r = received_list->elements;
    for (uint32_t i = 0; i < received_list->elementsCount; ++i)
    {
        EXPECT_TRUE(*list_r / 2 == *list_s);
        ++list_s;
        ++list_r;
    }
    erpc_free(send_list.elements);
    erpc_free(received_list->elements);
    erpc_free(received_list);
}

TEST(test_list, SendReceived2Int32)
{
    list_1_t *received_list, send_list;
    send_list.elementsCount = 10;
    send_list.elements = (list_0_t *)erpc_malloc(send_list.elementsCount * sizeof(list_0_t));
    list_0_t *list_0_t_s = send_list.elements;
    for (uint32_t i = 0; i < send_list.elementsCount; ++i)
    {
        list_0_t_s->elementsCount = (int32_t)(i + 1);
        list_0_t_s->elements = (int32_t *)erpc_malloc(list_0_t_s->elementsCount * sizeof(int32_t));
        int32_t *list_s = list_0_t_s->elements;
        for (uint32_t j = 0; j < list_0_t_s->elementsCount; ++j)
        {
            *list_s = (int32_t)j;
            ++list_s;
        }
        ++list_0_t_s;
    }

    received_list = sendReceived2Int32(&send_list);

    list_0_t_s = send_list.elements;
    list_0_t *list_0_t_r = received_list->elements;
    for (uint32_t i = 0; i < received_list->elementsCount; ++i)
    {
        int32_t *list_r, *list_s;
        list_s = list_0_t_s->elements;
        list_r = list_0_t_r->elements;
        for (uint32_t j = 0; j < list_0_t_r->elementsCount; ++j)
        {
            EXPECT_TRUE((*list_r) / 2 == *list_s);
            ++list_s;
            ++list_r;
        }
        erpc_free(list_0_t_s->elements);
        erpc_free(list_0_t_r->elements);
        ++list_0_t_s;
        ++list_0_t_r;
    }
    erpc_free(send_list.elements);
    erpc_free(received_list->elements);
    erpc_free(received_list);
}

// TEST(test_list, ReceivedInt32Length)
// {
//     int32_t *received_list, *send_received_list;
//     uint32_t listSize = 12;
//     send_received_list = (int32_t *)erpc_malloc(listSize * sizeof(int32_t));
//     received_list = (int32_t *)erpc_malloc(listSize * sizeof(int32_t));
//
//     int32_t *list_s = send_received_list;
//     for (uint32_t i = 0; i < listSize; ++i)
//     {
//         *list_s = (int32_t)i;
//         ++list_s;
//     }
//
//     receivedInt32Length(received_list, send_received_list, listSize);
//
//     list_s = send_received_list;
//     int32_t *list_r = received_list;
//     for (uint32_t i = 0; i < listSize; ++i)
//     {
//         EXPECT_TRUE(*list_r / 2 == *list_s);
//         ++list_s;
//         ++list_r;
//     }
//     erpc_free(received_list);
//     erpc_free(send_received_list);
// }
//
// TEST(test_list, ReceivedInt32Length2)
// {
//     int32_t *received_list, *send_received_list;
//     uint32_t listSize = 8;
//     send_received_list = (int32_t *)erpc_malloc(listSize * sizeof(int32_t));
//     received_list = (int32_t *)erpc_malloc(listSize * sizeof(int32_t));
//
//     int32_t *list_s = send_received_list;
//     for (uint32_t i = 0; i < listSize; ++i)
//     {
//         *list_s = (int32_t)i;
//         ++list_s;
//     }
//
//     receivedInt32Length2(received_list, send_received_list, listSize);
//
//     list_s = send_received_list;
//     int32_t *list_r = received_list;
//     for (uint32_t i = 0; i < listSize; ++i)
//     {
//         EXPECT_TRUE(*list_r / 2 == *list_s);
//         ++list_s;
//         ++list_r;
//     }
//     erpc_free(received_list);
//     erpc_free(send_received_list);
// }

TEST(test_list, SendReceivedEnum)
{
    list_2_t *received_list, send_list;
    send_list.elementsCount = 3;
    send_list.elements = (enumColor *)erpc_malloc(send_list.elementsCount * sizeof(enumColor));
    send_list.elements[0] = red;
    send_list.elements[1] = green;
    send_list.elements[2] = blue;

    received_list = sendReceivedEnum(&send_list);

    enumColor *list_s = send_list.elements;
    enumColor *list_r = received_list->elements;
    for (uint32_t i = 0; i < received_list->elementsCount; ++i)
    {
        EXPECT_TRUE(*list_r == *list_s);
        ++list_s;
        ++list_r;
    }
    erpc_free(send_list.elements);
    erpc_free(received_list->elements);
    erpc_free(received_list);
}

TEST(test_list, SendReceived2Enum)
{
    list_3_t *received_list, send_list;
    send_list.elementsCount = 3;
    send_list.elements = (list_2_t *)erpc_malloc(send_list.elementsCount * sizeof(list_2_t));
    list_2_t *list_2_t_s = send_list.elements;
    for (uint32_t i = 0; i < send_list.elementsCount; ++i)
    {
        list_2_t_s->elementsCount = (int32_t)(i + 1);
        list_2_t_s->elements = (enumColor *)erpc_malloc(list_2_t_s->elementsCount * sizeof(enumColor));
        enumColor *list_s = list_2_t_s->elements;
        for (uint32_t j = 0; j < list_2_t_s->elementsCount; ++j)
        {
            switch (j)
            {
                case 0:
                    *list_s = red;
                    break;
                case 1:
                    *list_s = green;
                    break;
                case 2:
                    *list_s = blue;
                    break;
            }
            ++list_s;
        }
        ++list_2_t_s;
    }

    received_list = sendReceived2Enum(&send_list);

    list_2_t_s = send_list.elements;
    list_2_t *list_2_t_r = received_list->elements;
    for (uint32_t i = 0; i < received_list->elementsCount; ++i)
    {
        enumColor *list_r, *list_s;
        list_s = list_2_t_s->elements;
        list_r = list_2_t_r->elements;
        for (uint32_t j = 0; j < list_2_t_r->elementsCount; ++j)
        {
            EXPECT_TRUE(*list_r == *list_s);
            ++list_s;
            ++list_r;
        }
        erpc_free(list_2_t_s->elements);
        erpc_free(list_2_t_r->elements);
        ++list_2_t_s;
        ++list_2_t_r;
    }
    erpc_free(send_list.elements);
    erpc_free(received_list->elements);
    erpc_free(received_list);
}

TEST(test_list, SendReceivedStruct)
{
    list_4_t *received_list, send_list;
    send_list.elementsCount = 10;
    send_list.elements = (C *)erpc_malloc(send_list.elementsCount * sizeof(C));
    C *list_s = send_list.elements;
    for (uint32_t i = 0; i < send_list.elementsCount; ++i)
    {
        list_s->m = (int32_t)i;
        list_s->n = (int32_t)i * 2;
        ++list_s;
    }

    received_list = sendReceivedStruct(&send_list);

    list_s = send_list.elements;
    C *list_r = received_list->elements;
    for (uint32_t i = 0; i < received_list->elementsCount; ++i)
    {
        EXPECT_TRUE((list_r->m) / 2 == list_s->m);
        EXPECT_TRUE((list_r->n) / 2 == list_s->n);
        ++list_s;
        ++list_r;
    }
    erpc_free(send_list.elements);
    erpc_free(received_list->elements);
    erpc_free(received_list);
}

TEST(test_list, SendReceived2Struct)
{
    list_5_t *received_list, send_list;
    send_list.elementsCount = 10;
    send_list.elements = (list_4_t *)erpc_malloc(send_list.elementsCount * sizeof(list_4_t));
    list_4_t *list_0_t_s = send_list.elements;
    for (uint32_t i = 0; i < send_list.elementsCount; ++i)
    {
        list_0_t_s->elementsCount = (int32_t)(i + 1);
        list_0_t_s->elements = (C *)erpc_malloc(list_0_t_s->elementsCount * sizeof(C));
        C *list_s = list_0_t_s->elements;
        for (uint32_t j = 0; j < list_0_t_s->elementsCount; ++j)
        {
            list_s->m = (int32_t)i;
            list_s->n = (int32_t)j;
            ++list_s;
        }
        ++list_0_t_s;
    }

    received_list = sendReceived2Struct(&send_list);

    list_0_t_s = send_list.elements;
    list_4_t *list_0_t_r = received_list->elements;
    for (uint32_t i = 0; i < received_list->elementsCount; ++i)
    {
        C *list_r, *list_s;
        list_s = list_0_t_s->elements;
        list_r = list_0_t_r->elements;
        for (uint32_t j = 0; j < list_0_t_r->elementsCount; ++j)
        {
            EXPECT_TRUE(list_r->m / 2 == list_s->m);
            EXPECT_TRUE(list_r->n / 2 == list_s->n);
            ++list_s;
            ++list_r;
        }
        erpc_free(list_0_t_s->elements);
        erpc_free(list_0_t_r->elements);
        ++list_0_t_s;
        ++list_0_t_r;
    }
    erpc_free(send_list.elements);
    erpc_free(received_list->elements);
    erpc_free(received_list);
}

TEST(test_list, SendReceivedString)
{
    list_6_t *received_list, send_list;
    char *compare_string = (char *)"Hello World!";
    send_list.elementsCount = 12;
    send_list.elements = (char **)erpc_malloc(send_list.elementsCount * sizeof(char *));
    char **list_s = send_list.elements;
    for (uint32_t i = 0; i < send_list.elementsCount; ++i)
    {
        *list_s = (char *)"Hello";
        ++list_s;
    }

    received_list = sendReceivedString(&send_list);
    char **list_r = received_list->elements;
    for (uint32_t i = 0; i < received_list->elementsCount; ++i)
    {
        EXPECT_STREQ(compare_string, *list_r);
        erpc_free(*list_r);
        ++list_r;
    }
    erpc_free(send_list.elements);
    erpc_free(received_list->elements);
    erpc_free(received_list);
}

TEST(test_list, SendReceived2String)
{
    char *compare_string = (char *)"Hello World!";
    list_7_t *received_list, send_list;
    send_list.elementsCount = 5;
    send_list.elements = (list_6_t *)erpc_malloc(send_list.elementsCount * sizeof(list_6_t));
    list_6_t *list_0_t_s = send_list.elements;
    for (uint32_t i = 0; i < send_list.elementsCount; ++i)
    {
        list_0_t_s->elementsCount = (int32_t)(i + 1);
        list_0_t_s->elements = (char **)erpc_malloc(list_0_t_s->elementsCount * sizeof(char *));
        char **list_s = list_0_t_s->elements;
        for (uint32_t j = 0; j < list_0_t_s->elementsCount; ++j)
        {
            *list_s = (char *)"Hello";
            ++list_s;
        }
        ++list_0_t_s;
    }

    received_list = sendReceived2String(&send_list);

    list_6_t *list_0_t_r = received_list->elements;
    list_0_t_s = send_list.elements;
    for (uint32_t i = 0; i < received_list->elementsCount; ++i)
    {
        char **list_r;
        list_r = list_0_t_r->elements;
        for (uint32_t j = 0; j < list_0_t_r->elementsCount; ++j)
        {
            EXPECT_STREQ(compare_string, *list_r);
            erpc_free(*list_r);
            ++list_r;
        }
        erpc_free(list_0_t_s->elements);
        erpc_free(list_0_t_r->elements);
        ++list_0_t_s;
        ++list_0_t_r;
    }
    erpc_free(send_list.elements);
    erpc_free(received_list->elements);
    erpc_free(received_list);
}

TEST(test_list, test_list_allDirection)
{
    list_8_t *send_list_a, *send_list_b, *send_list_e;
    list_8_t *expect_list_a, *expect_list_b, *expect_list_e;
    send_list_a = (list_8_t *)erpc_malloc(sizeof(list_8_t));
    send_list_b = (list_8_t *)erpc_malloc(sizeof(list_8_t));
    send_list_e = (list_8_t *)erpc_malloc(sizeof(list_8_t));
    expect_list_a = (list_8_t *)erpc_malloc(sizeof(list_8_t));
    expect_list_b = (list_8_t *)erpc_malloc(sizeof(list_8_t));
    expect_list_e = (list_8_t *)erpc_malloc(sizeof(list_8_t));
    send_list_a->elementsCount = send_list_b->elementsCount = send_list_e->elementsCount = 10;
    expect_list_a->elementsCount = expect_list_b->elementsCount = expect_list_e->elementsCount = 10;
    send_list_a->elements = (uint32_t *)erpc_malloc(send_list_a->elementsCount * sizeof(uint32_t));
    send_list_b->elements = (uint32_t *)erpc_malloc(send_list_b->elementsCount * sizeof(uint32_t));
    send_list_e->elements = (uint32_t *)erpc_malloc(send_list_e->elementsCount * sizeof(uint32_t));
    expect_list_a->elements = (uint32_t *)erpc_malloc(expect_list_a->elementsCount * sizeof(uint32_t));
    expect_list_b->elements = (uint32_t *)erpc_malloc(expect_list_b->elementsCount * sizeof(uint32_t));
    expect_list_e->elements = (uint32_t *)erpc_malloc(expect_list_e->elementsCount * sizeof(uint32_t));
    uint32_t *s_list_a = send_list_a->elements;
    uint32_t *s_list_b = send_list_b->elements;
    uint32_t *s_list_e = send_list_e->elements;
    uint32_t *e_list_a = expect_list_a->elements;
    uint32_t *e_list_b = expect_list_b->elements;
    uint32_t *e_list_e = expect_list_e->elements;
    for (uint32_t i = 0; i < send_list_a->elementsCount; ++i)
    {
        *s_list_a = *e_list_a = i;
        *s_list_b = *e_list_b = i * 2;
        *s_list_e = i * 6;
        *e_list_e = (*s_list_e) * 2;
        ++s_list_a;
        ++s_list_b;
        ++s_list_e;
        ++e_list_a;
        ++e_list_b;
        ++e_list_e;
    }

    test_list_allDirection(send_list_a, send_list_b, send_list_e);

    s_list_a = send_list_a->elements;
    s_list_b = send_list_b->elements;
    s_list_e = send_list_e->elements;
    e_list_a = expect_list_a->elements;
    e_list_b = expect_list_b->elements;
    e_list_e = expect_list_e->elements;
    for (uint32_t i = 0; i < send_list_a->elementsCount; ++i)
    {
        EXPECT_TRUE(*s_list_a == *e_list_a);
        EXPECT_TRUE(*s_list_b == *e_list_b);
        EXPECT_TRUE(*s_list_e == *e_list_e);
        ++s_list_a;
        ++s_list_b;
        ++s_list_e;
        ++e_list_a;
        ++e_list_b;
        ++e_list_e;
    }
    erpc_free(send_list_a->elements);
    erpc_free(send_list_b->elements);
    erpc_free(send_list_e->elements);
    erpc_free(expect_list_a->elements);
    erpc_free(expect_list_b->elements);
    erpc_free(expect_list_e->elements);
    erpc_free(send_list_a);
    erpc_free(send_list_b);
    erpc_free(send_list_e);
    erpc_free(expect_list_a);
    erpc_free(expect_list_b);
    erpc_free(expect_list_e);
}

/*TEST(test_list, test_list_allDirection)
{
    list_0_t *send_list_a, *send_list_b, *send_list_c, *send_list_d, *send_list_e;
    list_0_t *expect_list_a, *expect_list_b, *expect_list_c, *expect_list_d, *expect_list_e;
    send_list_a = (list_0_t *)erpc_malloc(sizeof(list_0_t));
    send_list_b = (list_0_t *)erpc_malloc(sizeof(list_0_t));
    send_list_c = (list_0_t *)erpc_malloc(sizeof(list_0_t));
    send_list_e = (list_0_t *)erpc_malloc(sizeof(list_0_t));
    expect_list_a = (list_0_t *)erpc_malloc(sizeof(list_0_t));
    expect_list_b = (list_0_t *)erpc_malloc(sizeof(list_0_t));
    expect_list_c = (list_0_t *)erpc_malloc(sizeof(list_0_t));
    expect_list_d = (list_0_t *)erpc_malloc(sizeof(list_0_t));
    expect_list_e = (list_0_t *)erpc_malloc(sizeof(list_0_t));
    send_list_a->elementsCount = send_list_b->elementsCount = send_list_e->elementsCount = 10;
    expect_list_a->elementsCount = expect_list_b->elementsCount = expect_list_c->elementsCount =
        expect_list_d->elementsCount = expect_list_e->elementsCount = 10;
    send_list_a->elements = (int32_t *)erpc_malloc(send_list_a->elementsCount * sizeof(int32_t));
    send_list_b->elements = (int32_t *)erpc_malloc(send_list_b->elementsCount * sizeof(int32_t));
    send_list_e->elements = (int32_t *)erpc_malloc(send_list_e->elementsCount * sizeof(int32_t));
    expect_list_a->elements = (int32_t *)erpc_malloc(expect_list_a->elementsCount * sizeof(int32_t));
    expect_list_b->elements = (int32_t *)erpc_malloc(expect_list_b->elementsCount * sizeof(int32_t));
    expect_list_c->elements = (int32_t *)erpc_malloc(expect_list_c->elementsCount * sizeof(int32_t));
    expect_list_d->elements = (int32_t *)erpc_malloc(expect_list_d->elementsCount * sizeof(int32_t));
    expect_list_e->elements = (int32_t *)erpc_malloc(expect_list_e->elementsCount * sizeof(int32_t));
    int32_t *s_list_a = send_list_a->elements;
    int32_t *s_list_b = send_list_b->elements;
    int32_t *s_list_c;
    int32_t *s_list_d;
    int32_t *s_list_e = send_list_e->elements;
    int32_t *e_list_a = expect_list_a->elements;
    int32_t *e_list_b = expect_list_b->elements;
    int32_t *e_list_c = expect_list_c->elements;
    int32_t *e_list_d = expect_list_d->elements;
    int32_t *e_list_e = expect_list_e->elements;
    for (uint32_t i = 0; i < send_list_a->elementsCount; ++i)
    {
        *s_list_a = *e_list_a = (int32_t)i;
        *s_list_b = *e_list_b = (int32_t)i * 2;
        *s_list_e = (int32_t)i * 6;
        *e_list_c = (int32_t)i * 4;
        *e_list_d = (int32_t)i * 5;
        *e_list_e = (*s_list_e) * 2;
        ++s_list_a;
        ++s_list_b;
        ++s_list_e;
        ++e_list_a;
        ++e_list_b;
        ++e_list_c;
        ++e_list_d;
        ++e_list_e;
    }

    test_list_allDirection(send_list_a, send_list_b, send_list_c, &send_list_d, send_list_e);

    s_list_a = send_list_a->elements;
    s_list_b = send_list_b->elements;
    s_list_c = send_list_c->elements;
    s_list_d = send_list_d->elements;
    s_list_e = send_list_e->elements;
    e_list_a = expect_list_a->elements;
    e_list_b = expect_list_b->elements;
    e_list_c = expect_list_c->elements;
    e_list_d = expect_list_d->elements;
    e_list_e = expect_list_e->elements;
    for (uint32_t i = 0; i < send_list_a->elementsCount; ++i)
    {
        EXPECT_TRUE(*s_list_a == *e_list_a);
        EXPECT_TRUE(*s_list_b == *e_list_b);
        EXPECT_TRUE(*s_list_c == *e_list_c);
        EXPECT_TRUE(*s_list_d == *e_list_d);
        EXPECT_TRUE(*s_list_e == *e_list_e);
        ++s_list_a;
        ++s_list_b;
        ++s_list_c;
        ++s_list_d;
        ++s_list_e;
        ++e_list_a;
        ++e_list_b;
        ++e_list_c;
        ++e_list_d;
        ++e_list_e;
    }
    free(send_list_a->elements);
    free(send_list_b->elements);
    free(send_list_c->elements);
    free(send_list_d->elements);
    free(send_list_e->elements);
    free(expect_list_a->elements);
    free(expect_list_b->elements);
    free(expect_list_c->elements);
    free(expect_list_d->elements);
    free(expect_list_e->elements);
    free(send_list_a);
    free(send_list_b);
    free(send_list_c);
    free(send_list_d);
    free(send_list_e);
    free(expect_list_a);
    free(expect_list_b);
    free(expect_list_c);
    free(expect_list_d);
    free(expect_list_e);
}*/

TEST(test_list, testLengthAnnotation)
{
    int32_t length = 5;
    int32_t *list = (int32_t *)erpc_malloc(length * sizeof(int32_t));
    int32_t *list_ptr = list;
    for (int i = 0; i < length; ++i)
    {
        *list_ptr = i + 1;
        ++list_ptr;
    }
    int32_t result = testLengthAnnotation(list, length);
    EXPECT_TRUE(result == 1);
    erpc_free(list);
}

TEST(test_list, testLengthAnnotationInStruct)
{
    listStruct myListStruct;
    myListStruct.len = 5;
    myListStruct.myList = (int32_t *)erpc_malloc(myListStruct.len * sizeof(int32_t));
    for (unsigned int i = 0; i < myListStruct.len; ++i)
    {
        myListStruct.myList[i] = (int32_t)i + 1;
    }
    int32_t result = testLengthAnnotationInStruct(&myListStruct);
    EXPECT_TRUE(result == 10);
    erpc_free(myListStruct.myList);
}

TEST(test_list, returnSentStructLengthAnnotation)
{
    listStruct myListStruct;
    myListStruct.len = 5;
    myListStruct.myList = (int32_t *)erpc_malloc(myListStruct.len * sizeof(int32_t));
    for (unsigned int i = 0; i < myListStruct.len; ++i)
    {
        myListStruct.myList[i] = (int32_t)i + 1;
    }
    listStruct *returnStruct = returnSentStructLengthAnnotation(&myListStruct);
    EXPECT_TRUE(myListStruct.len == returnStruct->len);
    for (unsigned int i = 0; i < returnStruct->len; ++i)
    {
        EXPECT_TRUE(myListStruct.myList[i] == returnStruct->myList[i]);
    }
    erpc_free(myListStruct.myList);
    erpc_free(returnStruct->myList);
    erpc_free(returnStruct);
}

TEST(test_list, sendGapAdvertisingData)
{
    gapAdvertisingData_t ad;
    ad.cNumAdStructures = 2;
    ad.aAdStructures = (gapAdStructure_t *)erpc_malloc(ad.cNumAdStructures * sizeof(gapAdStructure_t));

    gapAdStructure_t ad1;
    ad1.length = 3;
    ad1.adType = 5;
    ad1.aData = (uint8_t *)erpc_malloc(ad1.length * sizeof(uint8_t));
    for (int i = 0; i < ad1.length; ++i)
    {
        ad1.aData[i] = (i + 1) * 5;
    }
    ad.aAdStructures[0] = ad1;

    gapAdStructure_t ad2;
    ad2.length = 3;
    ad2.adType = 5;
    ad2.aData = (uint8_t *)erpc_malloc(ad2.length * sizeof(uint8_t));
    for (unsigned int i = 0; i < ad2.length; ++i)
    {
        ad2.aData[i] = ((int32_t)i + 1) * 5;
    }
    ad.aAdStructures[1] = ad2;

    int32_t result = sendGapAdvertisingData(&ad);

    EXPECT_TRUE(33 == result);

    erpc_free(ad1.aData);
    erpc_free(ad2.aData);
    erpc_free(ad.aAdStructures);
}
