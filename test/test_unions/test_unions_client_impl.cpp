/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "gtest.h"
#include "test.h"
#include <string.h>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Unit test Implementation code
////////////////////////////////////////////////////////////////////////////////

TEST(test_unions, testGenericCallback)
{
    gapGenericEvent_t event = {
        gInternalError_c,
        { gBleSuccess_c, gHciCommandStatus_c, 5 },
    };
    gapGenericEvent_t *newEvent = testGenericCallback(&event);
    EXPECT_TRUE(newEvent->eventType == gWhiteListSizeReady_c);
    erpc_free(newEvent);

    event.eventType = gRandomAddressReady_c;
    int x = 0xAA;
    for (int i = 0; i < 6; ++i)
    {
        event.eventData.aAddress[i] = x ^ 0xFF;
    }
    newEvent = testGenericCallback(&event);
    EXPECT_TRUE(newEvent->eventType == gTestCaseReturn_c);
    EXPECT_TRUE(newEvent->eventData.returnCode == 1);
    erpc_free(newEvent);

    event.eventType = gWhiteListSizeReady_c;
    event.eventData.whiteListSize = 100;
    newEvent = testGenericCallback(&event);
    EXPECT_TRUE(newEvent->eventType == gTestCaseReturn_c);
    // printf("Vaue of return code: %d\n", newEvent->eventData.returnCode);
    EXPECT_TRUE(newEvent->eventData.returnCode == 100);
    erpc_free(newEvent);
}

TEST(test_unions, testUnionLists)
{
    foo myFoo;
    myFoo.discriminator = orange;
    myFoo.bing.a.elements = (int32_t *)erpc_malloc(5 * sizeof(int32_t));
    myFoo.bing.a.elementsCount = 5;
    for (int i = 0; i < 5; ++i)
    {
        myFoo.bing.a.elements[i] = i + 1;
    }
    foo *returnFoo = sendMyFoo(&myFoo);
    EXPECT_TRUE(returnFoo->discriminator == returnVal);
    EXPECT_TRUE(returnFoo->bing.ret == 0xAA);
    erpc_free((void *)myFoo.bing.a.elements);
    erpc_free((void *)returnFoo);

    myFoo.discriminator = banana;
    myFoo.bing.x = 3;
    myFoo.bing.y = 4.0;
    returnFoo = sendMyFoo(&myFoo);

    EXPECT_TRUE(returnFoo->discriminator == papaya);
    EXPECT_TRUE(returnFoo->bing.x == 4);
    EXPECT_TRUE(returnFoo->bing.y == 3);

    erpc_free((void *)returnFoo);
}

TEST(test_unions, testNestedStructs)
{
    foo myFoo;
    myFoo.discriminator = apple;
    myFoo.bing.myFoobar.a = 2.0;
    myFoo.bing.myFoobar.rawString.dataLength = 10;
    myFoo.bing.myFoobar.rawString.data =
        (uint8_t *)erpc_malloc(myFoo.bing.myFoobar.rawString.dataLength * sizeof(uint8_t));
    for (int i = 0; i < 10; ++i)
    {
        myFoo.bing.myFoobar.rawString.data[i] = i + 1;
    }

    foo *returnFoo = sendMyFoo(&myFoo);
    EXPECT_TRUE(returnFoo->discriminator == returnVal);
    EXPECT_TRUE(returnFoo->bing.ret == 0xAA);
    erpc_free((void *)myFoo.bing.myFoobar.rawString.data);
    erpc_free((void *)returnFoo);
}

TEST(test_unions, testUnionAnn)
{
    unionType myFoo;
    fruit discriminator;
    discriminator = orange;
    myFoo.a.elements = (int32_t *)erpc_malloc(5 * sizeof(int32_t));
    myFoo.a.elementsCount = 5;
    for (int i = 0; i < 5; ++i)
    {
        myFoo.a.elements[i] = i + 1;
    }
    foo *returnFoo = sendMyUnion(discriminator, &myFoo);
    EXPECT_TRUE(returnFoo->discriminator == returnVal);
    EXPECT_TRUE(returnFoo->bing.ret == 0xAA);
    erpc_free((void *)myFoo.a.elements);
    erpc_free((void *)returnFoo);

    discriminator = banana;
    myFoo.x = 3;
    myFoo.y = 4.0;
    returnFoo = sendMyUnion(discriminator, &myFoo);

    EXPECT_TRUE(returnFoo->discriminator == papaya);
    EXPECT_TRUE(returnFoo->bing.x == 4);
    EXPECT_TRUE(returnFoo->bing.y == 3);

    erpc_free((void *)returnFoo);
}
