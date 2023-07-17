/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_simple_server.hpp"

#include "gtest.h"
#include "test_firstInterface.h"
#include "test_secondInterface_server.h"

////////////////////////////////////////////////////////////////////////////////
// Unit test Implementation code
////////////////////////////////////////////////////////////////////////////////

#define number 15
#define nestedCallsCount 10
volatile int j = 0;
volatile int numbers[number];
volatile bool enabled = false;
SecondInterface_service *svc;

TEST(test_arbitrator, FirstSendReceiveInt)
{
    for (int i = 0; i < number; i++)
    {
        firstSendInt(i);
    }
    for (int i = number - 1; i >= 0; i--)
    {
        int b = firstReceiveInt();
        EXPECT_EQ(i, b);
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
        EXPECT_EQ(i, b);
    }
}

TEST(test_arbitrator, NestedCallTest)
{
    stopSecondSide();
    while (!enabled)
    {
    };
    EXPECT_EQ(nestedCallTest(), nestedCallsCount * 2 - 1);
}

TEST(test_arbitrator, GetResultFromSecondSide)
{
    EXPECT_EQ(getResultFromSecondSide(), 0);
}

TEST(test_arbitrator, testCasesAreDone)
{
    testCasesAreDone();
}

void secondSendInt(int32_t a)
{
    numbers[j] = a;
    j++;
}

int32_t secondReceiveInt()
{
    j--;
    return numbers[j];
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
     */
    svc = new SecondInterface_service();

    /* Add services
     * Example: server->addService(svc);
     */
    server->addService(svc);
}

////////////////////////////////////////////////////////////////////////////////
// Remove service from server code
////////////////////////////////////////////////////////////////////////////////

void remove_services(erpc::SimpleServer *server)
{
    /* Remove services
     * Example: server->removeService (svc);
     */
    server->removeService(svc);
    /* Delete unused service
     */
    delete svc;
}
