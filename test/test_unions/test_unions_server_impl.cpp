/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_server_setup.h"

#include "test_server.h"
#include "test_unit_test_common_server.h"
#include "unit_test.h"
#include "unit_test_wrapped.h"

#include <stdlib.h>
#include <string.h>

ArithmeticService_service *svc;

////////////////////////////////////////////////////////////////////////////////
// Implementation of function code
////////////////////////////////////////////////////////////////////////////////

gapGenericEvent_t *testGenericCallback(const gapGenericEvent_t *event)
{
    gapGenericEvent_t *newEvent = (gapGenericEvent_t *)erpc_malloc(sizeof(gapGenericEvent_t));
    switch (event->eventType)
    {
        case gInternalError_c:
        {
            if (event->eventData.internalError.errorCode == gBleSuccess_c &&
                event->eventData.internalError.errorSource == gHciCommandStatus_c &&
                event->eventData.internalError.hciCommandOpcode == 5)
            {
                newEvent->eventType = gWhiteListSizeReady_c;
            }
            else
            {
                newEvent->eventType = gAdvertisingSetupFailed_c;
            }
            break;
        }
        case gRandomAddressReady_c:
        {
            int x = 0xAA;
            int success = 1;
            int i = 0;
            while (success && i < 6)
            {
                if (event->eventData.aAddress[i] != (x ^ 0xFF))
                {
                    success = false;
                }
                ++i;
            }
            newEvent->eventType = gTestCaseReturn_c;
            if (success)
            {
                newEvent->eventData.returnCode = 1;
            }
            else
            {
                newEvent->eventData.returnCode = 0;
            }
            break;
        }
        case gWhiteListSizeReady_c:
        {
            newEvent->eventType = gTestCaseReturn_c;
            if (100 == event->eventData.whiteListSize)
            {
                newEvent->eventData.returnCode = 100;
            }
            else
            {
                newEvent->eventData.returnCode = 0;
            }
            break;
        }
        case gPublicAddressRead_c:
        case gAdvertisingSetupFailed_c:
        case gAdvTxPowerLevelRead_c:
        default:
        {
        }
    }
    return newEvent;
}

foo *sendMyFoo(const foo *f)
{
    foo *newFoo = (foo *)erpc_malloc(sizeof(foo));
    switch (f->discriminator)
    {
        case apple:
        {
            for (uint32_t i = 0; i < f->bing.myFoobar.rawString.dataLength; ++i)
            {
                if ((i + 1) != f->bing.myFoobar.rawString.data[i])
                {
                    newFoo->discriminator = returnVal;
                    newFoo->bing.ret = 0x55;
                    break;
                }
            }
            newFoo->discriminator = returnVal;
            newFoo->bing.ret = 0xAA;
            erpc_free(f->bing.myFoobar.rawString.data);
            break;
        }
        case banana:
        {
            if ((f->bing.x == 3) && (f->bing.y == 4.0))
            {
                newFoo->discriminator = papaya;
                newFoo->bing.x = 4;
                newFoo->bing.y = 3;
            }
            else
            {
                newFoo->discriminator = papaya;
                newFoo->bing.x = 1;
                newFoo->bing.x = 1;
            }
            break;
        }
        case orange:
        {
            for (uint32_t i = 1; i <= f->bing.a.elementsCount; ++i)
            {
                // If data sent across is incorrect, return 0x55
                if ((int32_t)i != f->bing.a.elements[i])
                {
                    newFoo->discriminator = returnVal;
                    newFoo->bing.ret = 0x55;
                    break;
                }
            }
            // Else, data was sent properly, return 0xAA
            newFoo->discriminator = returnVal;
            newFoo->bing.ret = 0xAA;
            erpc_free(f->bing.a.elements);
            break;
        }
        default:
        {
            break;
        }
    }
    erpc_free((void *)f);
    return newFoo;
}

foo *sendMyUnion(fruit discriminator, const unionType *unionVariable)
{
    foo *newFoo = (foo *)erpc_malloc(sizeof(foo));
    switch (discriminator)
    {
        case apple:
        {
            for (uint32_t i = 0; i < unionVariable->myFoobar.rawString.dataLength; ++i)
            {
                if ((i + 1) != unionVariable->myFoobar.rawString.data[i])
                {
                    newFoo->discriminator = returnVal;
                    newFoo->bing.ret = 0x55;
                    break;
                }
            }
            newFoo->discriminator = returnVal;
            newFoo->bing.ret = 0xAA;
            break;
        }
        case banana:
        {
            if ((unionVariable->x == 3) && (unionVariable->y == 4.0))
            {
                newFoo->discriminator = papaya;
                newFoo->bing.x = 4;
                newFoo->bing.y = 3;
            }
            else
            {
                newFoo->discriminator = papaya;
                newFoo->bing.x = 1;
                newFoo->bing.x = 1;
            }
            break;
        }
        case orange:
        {
            for (uint32_t i = 1; i <= unionVariable->a.elementsCount; ++i)
            {
                // If data sent across is incorrect, return 0x55
                if ((int32_t)i != unionVariable->a.elements[i])
                {
                    newFoo->discriminator = returnVal;
                    newFoo->bing.ret = 0x55;
                    break;
                }
            }
            // Else, data was sent properly, return 0xAA
            newFoo->discriminator = returnVal;
            newFoo->bing.ret = 0xAA;
            break;
        }
        default:
        {
            break;
        }
    }
    return newFoo;
}

InnerList *testInnerList(const InnerList *il)
{
    InnerList *newList = (InnerList *)erpc_malloc(sizeof(InnerList));
    return newList;
}

////////////////////////////////////////////////////////////////////////////////
// Add service to server code
////////////////////////////////////////////////////////////////////////////////

void add_services(erpc::SimpleServer *server)
{
    // define services to add on heap
    // allocate on heap so service doesn't go out of scope at end of method
    svc = new ArithmeticService_service();

    // add services
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

#ifdef __cplusplus
extern "C" {
#endif
erpc_service_t service_test = NULL;
void add_services_to_server(erpc_server_t server)
{
    service_test = create_ArithmeticService_service();
    erpc_add_service_to_server(server, service_test);
}

void remove_services_from_server(erpc_server_t server)
{
    erpc_remove_service_from_server(server, service_test);
    destroy_ArithmeticService_service(service_test);
}

void remove_common_services_from_server(erpc_server_t server, erpc_service_t service)
{
    erpc_remove_service_from_server(server, service);
    destroy_Common_service(service);
}
#ifdef __cplusplus
}
#endif
