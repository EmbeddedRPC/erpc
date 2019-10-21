/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "gtest.h"
#include "test_core0.h"
#include "test_core1_server.h"

void callback3(int32_t param1, int32_t param2) {}

////////////////////////////////////////////////////////////////////////////////
// Unit test Implementation code
////////////////////////////////////////////////////////////////////////////////

TEST(test_callbacks, In_Out_table)
{
    callback1_t pCallback2_t = NULL;
    myFun(callback1, &pCallback2_t);

    EXPECT_TRUE(callback1 == *pCallback2_t);
}

TEST(test_callbacks, In_Out_withoutTable)
{
    callback2_t pCallback2_t = NULL;
    myFun2(callback3, &pCallback2_t);

    EXPECT_TRUE(callback3 == *pCallback2_t);
}
