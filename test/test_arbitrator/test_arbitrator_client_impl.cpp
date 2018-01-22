/*
 * The Clear BSD License
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 * that the following conditions are met:
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
#include "erpc_simple_server.h"
#include "test_firstInterface.h"
#include "test_secondInterface_server.h"

////////////////////////////////////////////////////////////////////////////////
// Unit test Implementation code
////////////////////////////////////////////////////////////////////////////////

#define number 15
#define nestedCallsCount 10
int i = 0;
int numbers[number];
volatile bool enabled = false;

TEST(test_arbitrator, FirstSendReceiveInt)
{
    for (int i = 0; i < number; i++)
    {
        firstSendInt(i);
    }
    for (int i = number - 1; i >= 0; i--)
    {
        int b = firstReceiveInt();
        EXPECT_TRUE(i == b);
    }
}

TEST(test_arbitrator, FirstSendReceiveInt2)
{
    for (int i = 0; i < number; i++)
    {
        firstSendInt(i);
    }
    for (int i = number - 1; i >= 0; i--)
    {
        int b = firstReceiveInt();
        EXPECT_TRUE(i == b);
    }
}

TEST(test_arbitrator, NestedCallTest)
{
    stopSecondSide();
    while (!enabled)
    {
    };
    EXPECT_TRUE(nestedCallTest() == nestedCallsCount * 2 - 1);
}

TEST(test_arbitrator, GetResultFromSecondSide)
{
    EXPECT_TRUE(getResultFromSecondSide() == 0);
}

void secondSendInt(int32_t a)
{
    numbers[i] = a;
    i++;
}

int32_t secondReceiveInt()
{
    i--;
    return numbers[i];
}

int32_t callFirstSide()
{
    static int _nestedCallsCount = 0;
    ++_nestedCallsCount;
    if (_nestedCallsCount < nestedCallsCount)
    {
        return callSecondSide() + 1;
    }
    else
    {
        return 0;
    }
}

void enableFirstSide()
{
    enabled = true;
}

void add_services(erpc::SimpleServer *server)
{
    /* Define services to add using dynamic memory allocation
    * Exapmle:ArithmeticService_service * svc = new ArithmeticService_service();
    */ // NOTE: possible memory leak? not ever deleting
    SecondInterface_service *svc = new SecondInterface_service();

    /* Add services
     * Example: server->addService(svc);
     */
    server->addService(svc);
}
