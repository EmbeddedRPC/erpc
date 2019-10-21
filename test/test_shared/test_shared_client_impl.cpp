/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "gtest.h"
#include "test.h"

////////////////////////////////////////////////////////////////////////////////
// Unit test Implementation code
////////////////////////////////////////////////////////////////////////////////

TEST(test_shared, sendReceiveBaseSharedStruct)
{
    BaseSharedStruct sm = { 4, 5 };
    BaseSharedStruct *_sm;
    _sm = sendReceiveBaseSharedStruct(&sm);
    EXPECT_TRUE(_sm == &sm);
}

TEST(test_shared, inoutBaseSharedStruct)
{
    BaseSharedStruct sm = { 4, 5 };
    BaseSharedStruct *_sm = &sm;
    inoutBaseSharedStruct(&_sm);
    EXPECT_TRUE(_sm == &sm);
}

/*TEST(test_shared, inoutStruct1)
{
    SharedStructMember sm = {4, 5};
    pB = sendReceiveInt(a);
    EXPECT_TRUE(b == pB);
}

TEST(test_shared, inoutStruct2)
{
    Colors a = green, pB, b = blue;
    pB = sendReceiveEnum(a);
    EXPECT_TRUE((int32_t)b == (int32_t)pB);
}

TEST(test_shared, inoutStruct3)
{
    Colors a = green, pB, b = blue;
    pB = sendReceiveEnum(a);
    EXPECT_TRUE((int32_t)b == (int32_t)pB);
}*/
