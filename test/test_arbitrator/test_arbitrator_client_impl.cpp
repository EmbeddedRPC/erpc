/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_simple_server.hpp"

#include "c_test_firstInterface_client.h"
#include "c_test_secondInterface_server.h"
#include "gtest.h"
#include "test_secondInterface_server.hpp"
#include "unit_test.h"
#include "unit_test_wrapped.h"

////////////////////////////////////////////////////////////////////////////////
// Unit test Implementation code
////////////////////////////////////////////////////////////////////////////////

using namespace erpc;

#define number 15
#define nestedCallsCount 10
int j = 0;
int numbers[number];
volatile bool enabled = false;
SecondInterface_service *svc;

void initInterfaces(erpc_client_t client)
{
    initFirstInterface_client(client);
}

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

class SecondInterface_server : public SecondInterface_interface
{
public:
    void secondSendInt(int32_t a) { ::secondSendInt(a); }

    int32_t secondReceiveInt(void)
    {
        int32_t result;
        result = ::secondReceiveInt();

        return result;
    }

    void quitSecondInterfaceServer(void) { ::quitSecondInterfaceServer(); }

    void enableFirstSide(void) { ::enableFirstSide(); }

    int32_t callFirstSide(void)
    {
        int32_t result;
        result = ::callFirstSide();

        return result;
    }
};

void add_services(erpc::SimpleServer *server)
{
    /* Define services to add using dynamic memory allocation
     * Exapmle:ArithmeticService_service * svc = new ArithmeticService_service();
     */
    svc = new SecondInterface_service(new SecondInterface_server());

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
    delete svc->getHandler();
    delete svc;
}
