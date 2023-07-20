/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "gtest.h"
#include "c_test_client.h"
#include "unit_test_wrapped.h"

////////////////////////////////////////////////////////////////////////////////
// Unit test Implementation code
////////////////////////////////////////////////////////////////////////////////

void initInterfaces(erpc_client_t client)
{
    initSharedService_client(client);
}

TEST(test_shared, sendReceiveBaseSharedStruct)
{
    BaseSharedStruct sm = { 4, 5 };
    BaseSharedStruct *_sm;
    _sm = sendReceiveBaseSharedStruct(&sm);
    EXPECT_EQ(_sm , &sm);
}

TEST(test_shared, inoutBaseSharedStruct)
{
    BaseSharedStruct sm = { 4, 5 };
    BaseSharedStruct *_sm = &sm;
    inoutBaseSharedStruct(&_sm);
    EXPECT_EQ(_sm , &sm);
}

/*TEST(test_shared, inoutStruct1)
{
    SharedStructMember sm = {4, 5};
    pB = sendReceiveInt(a);
    EXPECT_EQ(b , pB);
}

TEST(test_shared, inoutStruct2)
{
    Colors a = green, pB, b = blue;
    pB = sendReceiveEnum(a);
    EXPECT_EQ((int32_t)b , (int32_t)pB);
}

TEST(test_shared, inoutStruct3)
{
    Colors a = green, pB, b = blue;
    pB = sendReceiveEnum(a);
    EXPECT_EQ((int32_t)b , (int32_t)pB);
}*/
