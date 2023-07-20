/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "gtest.h"
#include "test_common.h"
#include "unit_test_wrapped.h"

////////////////////////////////////////////////////////////////////////////////
// Unit test Implementation code
////////////////////////////////////////////////////////////////////////////////

void initInterfaces(erpc_client_t client)
{
}

TEST(test_const, CheckConsts)
{
    EXPECT_EQ(a, 3);
    EXPECT_EQ((float)b, (float)3.14);
    EXPECT_STREQ("feedbabe", c);
    EXPECT_EQ((float)d, (float)3.14);
    EXPECT_EQ(x, 11);
    EXPECT_EQ(y, 20);

    EXPECT_EQ(mass, 100);
    EXPECT_EQ(accel, (float)-9.8);
}
