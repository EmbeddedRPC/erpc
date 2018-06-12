/*
 * The Clear BSD License
 * Copyright (c) 2014-2015, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 *  that the following conditions are met:
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
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
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
#include "test.h"
#include <string.h>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Unit test Implementation code
////////////////////////////////////////////////////////////////////////////////

int32_t int32A = 2;
int32_t int32B = -20;

char *stringA = (char *)"Hello";
char *stringB = (char *)"World!";

TEST(test_builtin, test_int32_in_out)
{
    int32_t c;
    test_int32_in(int32A);
    test_int32_out(&c);
    EXPECT_TRUE(int32A == c);
}

TEST(test_builtin, test_int32_inout)
{
    int32_t e = -6;
    for (int32_t i = -5; i <= 5; ++i)
    {
        test_int32_inout(&e);
        EXPECT_TRUE(i == e);
    }
}

TEST(test_builtin, test_int32_return)
{
    test_int32_in(int32A);
    test_int32_in2(int32B);
    int32_t r = test_int32_return();
    EXPECT_TRUE(int32A * int32B == r);
}

TEST(test_builtin, test_int32_allDirection)
{
    int32_t c;
    int32_t e = 7;

    int32_t r = test_int32_allDirection(int32A, int32B, &c, &e);

    EXPECT_TRUE(c == int32A);
    EXPECT_TRUE(14 == e);
    EXPECT_TRUE(int32A * int32B == r);
}

TEST(test_builtin, test_float_inout)
{
    float a = 3.14;
    float b;
    test_float_inout(a, &b);
    EXPECT_TRUE(a == b);
}

TEST(test_builtin, test_double_inout)
{
    double a = 3.14;
    double b;
    test_double_inout(a, &b);
    EXPECT_TRUE(a == b);
}

TEST(test_builtin, test_string_in_out)
{
    char *c = (char *)erpc_malloc(6 * sizeof(char));
    test_string_in(stringA);
    test_string_out(c);
    EXPECT_STREQ(stringA, c);
    erpc_free(c);
}

TEST(test_builtin, test_string_inout)
{
    char *e = (char *)erpc_malloc(13 * sizeof(char));
    strcpy(e, stringA);
    strcat(e, " ");
    test_string_in2(stringB);
    test_string_inout(e);
    EXPECT_STREQ("Hello World!", e);
    erpc_free(e);
}

TEST(test_builtin, test_string_return)
{
    char *r = test_string_return();
    EXPECT_STREQ("Hello World!", r);
    erpc_free(r);
}

TEST(test_builtin, test_string_allDirection)
{
    char *c = (char *)erpc_malloc(6 * sizeof(char));
    char *e = (char *)erpc_malloc(13 * sizeof(char));

    strcpy(e, stringA);
    strcat(e, " ");
    char *r = test_string_allDirection(stringA, stringB, c, e);

    EXPECT_STREQ(stringA, c);
    erpc_free(c);
    EXPECT_STREQ("Hello World!", e);
    erpc_free(e);
    EXPECT_STREQ("Hello World!", r);
    erpc_free(r);
}

/*TEST(test_builtin, test_string_empty)
{
    char *a = NULL;
    char *b = NULL;
    char c[21] = "";
    char *d = NULL;
    char e[21];
    e[0] = '\0';

    char *r = test_string_empty(a, b, c, &d, e);

    EXPECT_STREQ("", c);
    EXPECT_TRUE(d == NULL);
    EXPECT_STREQ("", e);
    EXPECT_TRUE(r == NULL);
}*/

TEST(test_builtin, StringParamTest1)
{
    const char *str = "Hello World!";
    int32_t result = sendHello(str);
    EXPECT_TRUE(result == 0);
}

TEST(test_builtin, StringParamTest2)
{
    const char *str1 = "String one.";
    const char *str2 = "String two.";
    int32_t result = sendTwoStrings(str1, str2);
    EXPECT_TRUE(result == 0);
}

TEST(test_builtin, StringReturnTest1)
{
    char *ret = returnHello();
    EXPECT_STREQ("Hello", ret);
    erpc_free(ret);
}
