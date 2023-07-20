/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_simple_server.hpp"

#include "c_test_firstInterface_server.h"
#include "c_test_secondInterface_client.h"
#include "test_firstInterface_server.hpp"
#include "unit_test_wrapped.h"

////////////////////////////////////////////////////////////////////////////////
// Unit test Implementation code
////////////////////////////////////////////////////////////////////////////////

using namespace erpc;

#define number 15
int i = 0;
int numbers[number];
FirstInterface_service *svc;


extern "C"
{
void initInterfaces(erpc_client_t client)
{
    initSecondInterface_client(client);
}

void firstSendInt(int32_t a)
{
    numbers[i] = a;
    i++;
}

int32_t firstReceiveInt()
{
    i--;
    return numbers[i];
}

int32_t nestedCallTest()
{
    return callFirstSide() + 1;
}

int32_t callSecondSide()
{
    return callFirstSide() + 1;
}
}

class FirstInterface_server : public FirstInterface_interface
{
public:
    void whenReady(void){};

    void firstSendInt(int32_t a) { firstSendInt(a); }

    int32_t firstReceiveInt(void) { return firstReceiveInt(); }

    void stopSecondSide(void) { stopSecondSide(); }

    int32_t getResultFromSecondSide(void) { return getResultFromSecondSide(); }

    void testCasesAreDone(void) { testCasesAreDone(); }

    void quitFirstInterfaceServer(void) { quitFirstInterfaceServer(); }

    int32_t nestedCallTest(void) { return nestedCallTest(); }

    int32_t callSecondSide(void) { return callSecondSide(); }
};

void add_services(erpc::SimpleServer *server)
{
    /* Define services to add using dynamic memory allocation
     * Exapmle:ArithmeticService_service * svc = new ArithmeticService_service();
     */
    svc = new FirstInterface_service(new FirstInterface_server());

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
