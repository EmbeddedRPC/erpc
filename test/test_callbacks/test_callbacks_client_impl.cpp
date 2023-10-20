/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "c_test_core0_client.h"
#include "gtest.h"
#include "unit_test_wrapped.h"

void callback2(int32_t param1, int32_t param2) {}

////////////////////////////////////////////////////////////////////////////////
// Unit test Implementation code
////////////////////////////////////////////////////////////////////////////////

void initInterfaces(erpc_client_t client)
{
    initClientCore0Services_client(client);
}

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
    EXPECT_EQ(12, myFun3(callback, 9, 3));

    callback = my_sub;
    EXPECT_EQ(6, myFun3(callback, 9, 3));

    callback = my_mul;
    EXPECT_EQ(27, myFun3(callback, 9, 3));

    callback = my_div;
    EXPECT_EQ(3, myFun3(callback, 9, 3));
}
