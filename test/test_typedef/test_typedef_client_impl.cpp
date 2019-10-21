/*
 * Copyright (c) 2014-2015, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "gtest.h"
#include "test.h"
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
// Unit test Implementation code
////////////////////////////////////////////////////////////////////////////////

TEST(test_typedef, SendReceiveInt)
{
    int32type a = 10, b = 2 * a + 1, pB;
    pB = sendReceiveInt(a);
    EXPECT_TRUE(b == pB);
}

TEST(test_typedef, SendReceiveEnum)
{
    Colors a = green, pB, b = blue;
    pB = sendReceiveEnum(a);
    EXPECT_TRUE((int32_t)b == (int32_t)pB);
}

TEST(test_typedef, SendReceiveStruct)
{
    B *a = (B *)erpc_malloc(sizeof(B)), *pB, *b = (B *)erpc_malloc(sizeof(B));
    a->m = 1;
    a->n = 4;
    b->m = 2 * a->m;
    b->n = 2 + a->n;
    pB = sendReceiveStruct(a);
    EXPECT_TRUE(b->m == pB->m);
    EXPECT_TRUE(b->n == pB->n);
    erpc_free(a);
    erpc_free(b);
    erpc_free(pB);
}

TEST(test_typedef, SendReceiveListType)
{
    ListType *received_list, sent_list;
    sent_list.elementsCount = 12;
    sent_list.elements = (int32_t *)erpc_malloc(sent_list.elementsCount * sizeof(int32_t));
    int32_t *list_p = sent_list.elements;
    for (uint32_t i = 0; i < sent_list.elementsCount; ++i)
    {
        *list_p = (int32_t)i;
        ++list_p;
    }

    received_list = sendReceiveListType(&sent_list);

    list_p = sent_list.elements;
    int32_t *list_p2 = received_list->elements;
    for (uint32_t i = 0; i < received_list->elementsCount; ++i)
    {
        EXPECT_TRUE(*list_p2 / 2 == *list_p);
        ++list_p;
        ++list_p2;
    }
    erpc_free(sent_list.elements);
    erpc_free(received_list->elements);
    erpc_free(received_list);
}

TEST(test_typedef, SendReceiveString)
{
    newString send = (newString) "Hello";
    newString result = (newString) "Hello World!";
    newString received = sendReceiveString(send);
    EXPECT_STREQ(result, received);
    erpc_free(received);
}

TEST(test_typedef, SendReceive2ListType)
{
    ListType2 *received_list, send_list;
    send_list.elementsCount = 10;
    send_list.elements = (ListType *)erpc_malloc(send_list.elementsCount * sizeof(ListType));
    ListType *list_0_s = send_list.elements;
    for (uint32_t i = 0; i < send_list.elementsCount; ++i)
    {
        list_0_s->elementsCount = (int32_t)(i + 1);
        list_0_s->elements = (int32_t *)erpc_malloc(list_0_s->elementsCount * sizeof(int32_t));
        int32_t *list_s = list_0_s->elements;
        for (uint32_t j = 0; j < list_0_s->elementsCount; ++j)
        {
            *list_s = (int32_t)j;
            ++list_s;
        }
        ++list_0_s;
    }

    received_list = sendReceive2ListType(&send_list);

    list_0_s = send_list.elements;
    ListType *list_0_r = received_list->elements;
    for (uint32_t i = 0; i < received_list->elementsCount; ++i)
    {
        int32_t *list_r, *list_s;
        list_s = list_0_s->elements;
        list_r = list_0_r->elements;
        for (uint32_t j = 0; j < list_0_r->elementsCount; ++j)
        {
            EXPECT_TRUE((*list_r) / 2 == *list_s);
            ++list_s;
            ++list_r;
        }
        erpc_free(list_0_s->elements);
        erpc_free(list_0_r->elements);
        ++list_0_s;
        ++list_0_r;
    }
    erpc_free(received_list->elements);
    erpc_free(send_list.elements);
    erpc_free(received_list);
}
