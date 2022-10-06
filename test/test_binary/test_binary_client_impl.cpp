/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "gtest.h"
#include "test.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Unit test Implementation code
////////////////////////////////////////////////////////////////////////////////

/*TEST(test_binary, sendReceiveBinary)
{
    binary *result, send;
    send.dataLength = 5;
    send.data = (uint8_t *)erpc_malloc(send.dataLength * sizeof(uint8_t));
    for (uint8_t i = 0; i < send.dataLength; ++i)
    {
        send.data[i] = i;
    }
    result = sendReceiveBinary(&send);
    for (uint8_t i = 0; i < result->dataLength; ++i)
    {
        EXPECT_EQ(send.data[i] , result->data[i]);
    }
    free(send.data);
    free(result->data);
    free(result);
}*/

TEST(test_binary, sendBinary)
{
    binary_t send;
    send.dataLength = 5;
    send.data = (uint8_t *)erpc_malloc(send.dataLength * sizeof(uint8_t));
    for (uint8_t i = 0; i < send.dataLength; ++i)
    {
        send.data[i] = i;
    }
    sendBinary(&send);
    erpc_free(send.data);
}

/*TEST(test_binary, receiveBinary)
{
    binary *result;
    result = receiveBinary();
    for (uint8_t i = 0; i < result->dataLength; ++i)
    {
        EXPECT_EQ(i , result->data[i]);
    }
    free(result->data);
    free(result);
}*/

TEST(test_binary, test_binary_allDirection)
{
    binary_t a;
    binary_t b;
    binary_t e;

    a.dataLength = b.dataLength = e.dataLength = 5;
    a.data = (uint8_t *)erpc_malloc(a.dataLength * sizeof(uint8_t));
    b.data = (uint8_t *)erpc_malloc(b.dataLength * sizeof(uint8_t));
    e.data = (uint8_t *)erpc_malloc(e.dataLength * sizeof(uint8_t));
    for (uint8_t i = 0; i < a.dataLength; ++i)
    {
        a.data[i] = i;
        b.data[i] = i * 2;
        e.data[i] = i;
    }

    test_binary_allDirection(&a, &b, &e);

    for (uint8_t i = 0; i < 5; ++i)
    {
        EXPECT_EQ(e.data[i], a.data[i] * b.data[i]);
    }

    erpc_free(a.data);
    erpc_free(b.data);
    erpc_free(e.data);
}

/*TEST(test_binary, test_binary_allDirection)
{
    binary a;
    binary b;
    binary c;
    binary *d;
    binary e;

    a.dataLength = b.dataLength = e.dataLength = 5;
    a.data = (uint8_t *)erpc_malloc(a.dataLength * sizeof(uint8_t));
    b.data = (uint8_t *)erpc_malloc(b.dataLength * sizeof(uint8_t));
    e.data = (uint8_t *)erpc_malloc(e.dataLength * sizeof(uint8_t));
    for (uint8_t i = 0; i < a.dataLength; ++i)
    {
        a.data[i] = i;
        b.data[i] = i * 2;
        e.data[i] = i;
    }

    test_binary_allDirection(&a, &b, &c, &d, &e);

    for (uint8_t i = 0; i < 5; ++i)
    {
        EXPECT_EQ(a.data[i] , c.data[i]);
        EXPECT_EQ(b.data[i] , d->data[i]);
        EXPECT_EQ(e.data[i] , 4 * i);
    }

    free(a.data);
    free(b.data);
    free(c.data);
    free(d->data);
    free(d);
    free(e.data);
}*/
