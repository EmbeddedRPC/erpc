/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "gtest.h"
#include "test.h"

////////////////////////////////////////////////////////////////////////////////
// Unit test Implementation code
////////////////////////////////////////////////////////////////////////////////

TEST(test_const, CheckConsts)
{
    EXPECT_TRUE(a == 3);
    EXPECT_TRUE((float)b == (float)3.14);
    EXPECT_STREQ("feedbabe", c);
    EXPECT_TRUE((float)d == (float)3.14);
    EXPECT_TRUE(x == 11);
    EXPECT_TRUE(y == 20);

    EXPECT_TRUE(mass == 100);
    EXPECT_TRUE(accel == (float)-9.8);
}
