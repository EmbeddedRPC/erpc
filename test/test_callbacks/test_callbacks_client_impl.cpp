/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "gtest.h"
#include "test_core0.h"
#include "test_core1_server.h"

void callback2(int32_t param1, int32_t param2) {}

////////////////////////////////////////////////////////////////////////////////
// Unit test Implementation code
////////////////////////////////////////////////////////////////////////////////

TEST(test_callbacks, In_Out_table_1)
{
    callback1_t pCallback1_out = NULL;
    myFun(callback1a, &pCallback1_out);

    EXPECT_TRUE(callback1a == *pCallback1_out);
}

TEST(test_callbacks, In_Out_table_2)
{
    callback1_t pCallback1_out = NULL;
    myFun(callback1b, &pCallback1_out);

    EXPECT_TRUE(callback1b == *pCallback1_out);
}

TEST(test_callbacks, In_Out_withoutTable)
{
    callback2_t pCallback2_out = NULL;
    myFun2(callback2, &pCallback2_out);

    EXPECT_TRUE(callback2 == *pCallback2_out);
}

TEST(test_callbacks, Common_Callback)
{
    callback3_t callback = my_add;
    EXPECT_TRUE(12 == myFun3(callback, 9, 3));

    callback = my_sub;
    EXPECT_TRUE(6 == myFun3(callback, 9, 3));

    callback = my_mul;
    EXPECT_TRUE(27 == myFun3(callback, 9, 3));

    callback = my_div;
    EXPECT_TRUE(3 == myFun3(callback, 9, 3));
}