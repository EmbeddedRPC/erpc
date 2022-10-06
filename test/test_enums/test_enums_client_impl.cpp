/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "gtest.h"
#include "test.h"

////////////////////////////////////////////////////////////////////////////////
// Unit test Implementation code
////////////////////////////////////////////////////////////////////////////////

enumColor enumColorA = green;
enumColor enumColorB = red;

TEST(test_enum, test_enumColor_in_out)
{
    enumColor c;
    test_enumColor_in(enumColorA);
    test_enumColor_out(&c);
    EXPECT_EQ(enumColorA, c);
}

TEST(test_enum, test_enumColor_inout)
{
    enumColor e = enumColorA;
    test_enumColor_inout(&e);
    EXPECT_EQ(enumColorB, e);
}

TEST(test_enum, test_enumColor_return)
{
    test_enumColor_in(enumColorB);
    enumColor r = test_enumColor_return();
    EXPECT_EQ(enumColorA, r);
}

TEST(test_enum, test_enumColor_allDirection)
{
    enumColor c;
    enumColor e = enumColorA;

    enumColor r = test_enumColor_allDirection(enumColorA, enumColorB, &c, &e);

    EXPECT_EQ(enumColorA, c);
    EXPECT_EQ(enumColorB, e);
    EXPECT_EQ(enumColorA, r);
}

TEST(test_enum, test_enumColor2_allDirection)
{
    enumColor2 a = pink;
    enumColor2 b = yellow;
    enumColor2 c;
    enumColor2 e = a;

    enumColor2 r = test_enumColor2_allDirection(a, b, &c, &e);

    EXPECT_EQ(a, c);
    EXPECT_EQ(b, e);
    EXPECT_EQ(a, r);
}

TEST(test_enum, test_enumErrorCode_allDirection)
{
    enumErrorCode a = ERROR_NONE;
    enumErrorCode b = ERROR_UNKNOWN;
    enumErrorCode c;
    enumErrorCode e = a;

    enumErrorCode r = test_enumErrorCode_allDirection(a, b, &c, &e);

    EXPECT_EQ(a, c);
    EXPECT_EQ(b, e);
    EXPECT_EQ(a, r);
}
