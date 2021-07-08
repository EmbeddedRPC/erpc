/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_simple_server.h"

#include "test_firstInterface_server.h"
#include "test_secondInterface.h"

////////////////////////////////////////////////////////////////////////////////
// Unit test Implementation code
////////////////////////////////////////////////////////////////////////////////

#define number 15
int i = 0;
int numbers[number];
FirstInterface_service *svc;

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

void add_services(erpc::SimpleServer *server)
{
    /* Define services to add using dynamic memory allocation
     * Exapmle:ArithmeticService_service * svc = new ArithmeticService_service();
     */
    svc = new FirstInterface_service();

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
