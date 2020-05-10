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

enumColor enumColorA = green;
enumColor enumColorB = red;

TEST(test_enum, test_enumColor_in_out)
{
    enumColor c;
    test_enumColor_in(enumColorA);
    test_enumColor_out(&c);
    EXPECT_TRUE(enumColorA == c);
}

TEST(test_enum, test_enumColor_inout)
{
    enumColor e = enumColorA;
    test_enumColor_inout(&e);
    EXPECT_TRUE(enumColorB == e);
}

TEST(test_enum, test_enumColor_return)
{
    test_enumColor_in(enumColorB);
    enumColor r = test_enumColor_return();
    EXPECT_TRUE(enumColorA == r);
}

TEST(test_enum, test_enumColor_allDirection)
{
    enumColor c;
    enumColor e = enumColorA;

    enumColor r = test_enumColor_allDirection(enumColorA, enumColorB, &c, &e);

    EXPECT_TRUE(enumColorA == c);
    EXPECT_TRUE(enumColorB == e);
    EXPECT_TRUE(enumColorA == r);
}

TEST(test_enum, test_enumColor2_allDirection)
{
    enumColor2 a = pink;
    enumColor2 b = yellow;
    enumColor2 c;
    enumColor2 e = a;

    enumColor2 r = test_enumColor2_allDirection(a, b, &c, &e);

    EXPECT_TRUE(a == c);
    EXPECT_TRUE(b == e);
    EXPECT_TRUE(a == r);
}
