/*
 * The Clear BSD License
 * Copyright 2017 NXP
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

////////////////////////////////////////////////////////////////////////////////
// Unit test Implementation code
////////////////////////////////////////////////////////////////////////////////

TEST(test_shared, sendReceiveBaseSharedStruct)
{
    BaseSharedStruct sm = {4, 5};
    BaseSharedStruct *_sm;
    _sm = sendReceiveBaseSharedStruct(&sm);
    EXPECT_TRUE(_sm == &sm);
}

TEST(test_shared, inoutBaseSharedStruct)
{
    BaseSharedStruct sm = {4, 5};
    BaseSharedStruct *_sm = &sm;
    inoutBaseSharedStruct(&_sm);
    EXPECT_TRUE(_sm == &sm);
}

/*TEST(test_shared, inoutStruct1)
{
    SharedStructMember sm = {4, 5};
    pB = sendReceiveInt(a);
    EXPECT_TRUE(b == pB);
}

TEST(test_shared, inoutStruct2)
{
    Colors a = green, pB, b = blue;
    pB = sendReceiveEnum(a);
    EXPECT_TRUE((int32_t)b == (int32_t)pB);
}

TEST(test_shared, inoutStruct3)
{
    Colors a = green, pB, b = blue;
    pB = sendReceiveEnum(a);
    EXPECT_TRUE((int32_t)b == (int32_t)pB);
}*/
