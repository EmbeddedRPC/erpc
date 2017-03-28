/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "gtest.h"
#include "test_enums.h"

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

TEST(test_enum, test_enumColor_in2_outbyref)
{
    enumColor d;
    test_enumColor_in2(enumColorB);
    test_enumColor_outbyref(&d);
    EXPECT_TRUE(enumColorB == d);
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
    enumColor d;
    enumColor e = enumColorA;

    enumColor r = test_enumColor_allDirection(enumColorA, enumColorB, &c, &d, &e);

    EXPECT_TRUE(enumColorA == c);
    EXPECT_TRUE(enumColorB == d);
    EXPECT_TRUE(enumColorB == e);
    EXPECT_TRUE(enumColorA == r);
}

TEST(test_enum, test_enumColor2_allDirection)
{
    enumColor2 a = pink;
    enumColor2 b = yellow;
    enumColor2 c;
    enumColor2 d;
    enumColor2 e = a;

    enumColor2 r = test_enumColor2_allDirection(a, b, &c, &d, &e);

    EXPECT_TRUE(a == c);
    EXPECT_TRUE(b == d);
    EXPECT_TRUE(b == e);
    EXPECT_TRUE(a == r);
}

TEST(test_enum, getAnonymousEnum)
{
    AnonymousEnum a = aRed;
    AnonymousEnum r = getAnonymousEnum(a);
    EXPECT_TRUE(aOrange == r);
}
