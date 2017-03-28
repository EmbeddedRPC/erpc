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

#include "erpc_server_setup.h"
#include "test_unions_server.h"
#include "unit_test.h"
#include "unit_test_wrapped.h"
#include <stdlib.h>
#include <string.h>

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
                if (i != f->bing.a.elements[i])
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
    // NOTE: possible memory leak? not ever deleting
    ArithmeticService_service *svc = new ArithmeticService_service();

    // add services
    server->addService(svc);
}

#ifdef __cplusplus
extern "C" {
#endif
void add_services_to_server()
{
    erpc_add_service_to_server(create_ArithmeticService_service());
}
#ifdef __cplusplus
}
#endif
