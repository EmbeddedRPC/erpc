/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
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
#include "test.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Unit test Implementation code
////////////////////////////////////////////////////////////////////////////////

/*TEST(test_binary, sendReceiveBinary)
{
    binary *result, send;
    send.dataLength = 5;
    send.data = (uint8_t *)erpc_malloc(send.dataLength * sizeof(uint8_t));
    for (uint8_t i = 0; i < send.dataLength; ++i)
    {
        send.data[i] = i;
    }
    result = sendReceiveBinary(&send);
    for (uint8_t i = 0; i < result->dataLength; ++i)
    {
        EXPECT_TRUE(send.data[i] == result->data[i]);
    }
    free(send.data);
    free(result->data);
    free(result);
}*/

TEST(test_binary, sendBinary)
{
    binary_t send;
    send.dataLength = 5;
    send.data = (uint8_t *)erpc_malloc(send.dataLength * sizeof(uint8_t));
    for (uint8_t i = 0; i < send.dataLength; ++i)
    {
        send.data[i] = i;
    }
    sendBinary(&send);
    erpc_free(send.data);
}

/*TEST(test_binary, receiveBinary)
{
    binary *result;
    result = receiveBinary();
    for (uint8_t i = 0; i < result->dataLength; ++i)
    {
        EXPECT_TRUE(i == result->data[i]);
    }
    free(result->data);
    free(result);
}*/

TEST(test_binary, test_binary_allDirection)
{
    binary_t a;
    binary_t b;
    binary_t e;

    a.dataLength = b.dataLength = e.dataLength = 5;
    a.data = (uint8_t *)erpc_malloc(a.dataLength * sizeof(uint8_t));
    b.data = (uint8_t *)erpc_malloc(b.dataLength * sizeof(uint8_t));
    e.data = (uint8_t *)erpc_malloc(e.dataLength * sizeof(uint8_t));
    for (uint8_t i = 0; i < a.dataLength; ++i)
    {
        a.data[i] = i;
        b.data[i] = i * 2;
        e.data[i] = i;
    }

    test_binary_allDirection(&a, &b, &e);

    for (uint8_t i = 0; i < 5; ++i)
    {
        EXPECT_TRUE(e.data[i] == a.data[i] * b.data[i]);
    }

    erpc_free(a.data);
    erpc_free(b.data);
    erpc_free(e.data);
}

/*TEST(test_binary, test_binary_allDirection)
{
    binary a;
    binary b;
    binary c;
    binary *d;
    binary e;

    a.dataLength = b.dataLength = e.dataLength = 5;
    a.data = (uint8_t *)erpc_malloc(a.dataLength * sizeof(uint8_t));
    b.data = (uint8_t *)erpc_malloc(b.dataLength * sizeof(uint8_t));
    e.data = (uint8_t *)erpc_malloc(e.dataLength * sizeof(uint8_t));
    for (uint8_t i = 0; i < a.dataLength; ++i)
    {
        a.data[i] = i;
        b.data[i] = i * 2;
        e.data[i] = i;
    }

    test_binary_allDirection(&a, &b, &c, &d, &e);

    for (uint8_t i = 0; i < 5; ++i)
    {
        EXPECT_TRUE(a.data[i] == c.data[i]);
        EXPECT_TRUE(b.data[i] == d->data[i]);
        EXPECT_TRUE(e.data[i] == 4 * i);
    }

    free(a.data);
    free(b.data);
    free(c.data);
    free(d->data);
    free(d);
    free(e.data);
}*/
