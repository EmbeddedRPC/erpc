#!/usr/bin/python

# Copyright 2021-2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause

import pytest

from .service.erpc_outputs.test import common
from .service.erpc_outputs.test.client import ArithmeticServiceClient
from .service.erpc_outputs.test.interface import IArithmeticService
from .service.erpc_outputs.test.server import ArithmeticServiceService


###############################################################################
# Variables
###############################################################################


###############################################################################
# Fixtures
###############################################################################
@pytest.fixture()
def client(client_manager):
    if client_manager is None:
        pytest.skip()

    return ArithmeticServiceClient(client_manager)


@pytest.fixture(scope="module")
def services():
    return [ArithmeticServiceService(UnionsTestServiceHandler())]


###############################################################################
# Services
###############################################################################
class UnionsTestServiceHandler(IArithmeticService):
    def testGenericCallback(self, event):
        newEvent = common.gapGenericEvent_t(eventType=common.gapGenericEventType_t.gInternalError_c)
        newEvent.eventData.internalError = common.gapInternalError_t(errorCode=common.bleResult_t.gBleSuccess_c,
                                                                     errorSource=common.gapInternalErrorSource_t.gHciCommandStatus_c,
                                                                     hciCommandOpcode=5)
        if event.eventType == common.gapGenericEventType_t.gInternalError_c:
            if (event.eventData.internalError.errorCode == common.bleResult_t.gBleSuccess_c and
                    event.eventData.internalError.errorSource == common.gapInternalErrorSource_t.gHciCommandStatus_c and
                    event.eventData.internalError.hciCommandOpcode == 5):
                newEvent.eventType = common.gapGenericEventType_t.gWhiteListSizeReady_c
                newEvent.eventData.whiteListSize = 0
            else:
                newEvent.eventType = common.gapGenericEventType_t.gAdvertisingSetupFailed_c
                newEvent.eventData.whiteListSize = 0
        elif event.eventType == common.gapGenericEventType_t.gRandomAddressReady_c:
            newEvent.eventType = common.gapGenericEventType_t.gTestCaseReturn_c
            newEvent.eventData.returnCode = 1
            for i in range(common.gcBleDeviceAddressSize_c):
                if (event.eventData.aAddress[i] != (0xAA ^ 0xFF)):
                    newEvent.eventData.returnCode = 0
        elif event.eventType == common.gapGenericEventType_t.gWhiteListSizeReady_c:
            newEvent.eventType = common.gapGenericEventType_t.gTestCaseReturn_c
            if (event.eventData.whiteListSize == 100):
                newEvent.eventData.returnCode = 100
            else:
                newEvent.eventData.returnCode = 0
        elif event.eventType == common.gapGenericEventType_t.gPublicAddressRead_c:
            pass
        elif event.eventType == common.gapGenericEventType_t.gAdvertisingSetupFailed_c:
            pass
        elif event.eventType == common.gapGenericEventType_t.gAdvTxPowerLevelRead_c:
            pass
        return newEvent

    def sendMyFoo(self, f):
        newFoo = common.foo()
        if f.discriminator == common.fruit.apple:
            newFoo.discriminator = common.fruit.returnVal
            newFoo.bing.myFoobar = common.foobar(a=float(0), rawString=None)
            newFoo.bing.ret = 0xAA
            for i in range(len(f.bing.myFoobar.rawString)):
                if (f.bing.myFoobar.rawString[i] != (i + 1)):
                    newFoo.bing.ret = 0x55
        elif f.discriminator == common.fruit.banana:
            newFoo.discriminator = common.fruit.papaya
            if (f.bing.x == 3) and (f.bing.y == float(4.0)):
                newFoo.bing.x = 4
                newFoo.bing.y = 3
            else:
                newFoo.bing.x = 1
                newFoo.bing.y = 1
        elif f.discriminator == common.fruit.orange:
            newFoo.discriminator = common.fruit.returnVal
            newFoo.bing.ret = 0xAA
            for i in range(len(f.bing.a)):
                if (f.bing.a[i] != (i + 1)):
                    newFoo.bing.ret = 0x55
        return newFoo

    def sendMyUnion(self, discriminator, unionVariable):
        newFoo = common.foo()
        if discriminator == common.fruit.apple:
            newFoo.discriminator = common.fruit.returnVal
            newFoo.bing.myFoobar = common.foobar(a=float(0), rawString=None)
            newFoo.bing.ret = 0xAA
            for i in range(len(unionVariable.myFoobar.rawString)):
                if (unionVariable.myFoobar.rawString[i] != (i + 1)):
                    newFoo.bing.ret = 0x55
        elif discriminator == common.fruit.banana:
            newFoo.discriminator = common.fruit.papaya
            if (unionVariable.x == 3) and (unionVariable.y == float(4.0)):
                newFoo.bing.x = 4
                newFoo.bing.y = 3
            else:
                newFoo.bing.x = 1
                newFoo.bing.y = 1
        elif discriminator == common.fruit.orange:
            newFoo.discriminator = common.fruit.returnVal
            newFoo.bing.ret = 0xAA
            for i in range(len(unionVariable.a)):
                if (unionVariable.a[i] != (i + 1)):
                    newFoo.bing.ret = 0x55
        return newFoo


###############################################################################
# Handlers
###############################################################################
def test_TestUnionsServiceID():
    assert 2 == IArithmeticService.SERVICE_ID


def test_GenericCallback(client):
    event = common.gapGenericEvent_t(eventType=common.gapGenericEventType_t.gInternalError_c)
    event.eventData.internalError = common.gapInternalError_t(errorCode=common.bleResult_t.gBleSuccess_c,
                                                              errorSource=common.gapInternalErrorSource_t.gHciCommandStatus_c,
                                                              hciCommandOpcode=5)
    newEvent = common.gapGenericEvent_t()
    # print(event)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    newEvent = client.testGenericCallback(event)
    assert newEvent.eventType == common.gapGenericEventType_t.gWhiteListSizeReady_c

    event.eventType = common.gapGenericEventType_t.gRandomAddressReady_c
    event.eventData.aAddress = [0 for a in range(common.gcBleDeviceAddressSize_c)]
    for x in range(common.gcBleDeviceAddressSize_c):
        event.eventData.aAddress[x] = (0xAA ^ 0xFF)
    print('\r\neRPC request is sent to the server')
    newEvent = client.testGenericCallback(event)
    assert newEvent.eventType == common.gapGenericEventType_t.gTestCaseReturn_c
    assert newEvent.eventData.returnCode == 1

    event.eventType = common.gapGenericEventType_t.gWhiteListSizeReady_c
    event.eventData.whiteListSize = 100
    print('\r\neRPC request is sent to the server')
    newEvent = client.testGenericCallback(event)
    assert newEvent.eventType == common.gapGenericEventType_t.gTestCaseReturn_c
    assert newEvent.eventData.returnCode == 100


def test_UnionLists(client):
    myFoo = common.foo()
    returnFoo = common.foo()
    myFoo.discriminator = common.fruit.orange
    myFoo.bing.a = [a + 1 for a in range(5)]
    # send request to the server
    print('\r\neRPC request is sent to the server')
    returnFoo = client.sendMyFoo(myFoo)
    assert returnFoo.discriminator == common.fruit.returnVal
    assert returnFoo.bing.ret == 0xAA

    myFoo.discriminator = common.fruit.banana
    myFoo.bing.x = 3
    myFoo.bing.y = float(4.0)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    returnFoo = client.sendMyFoo(myFoo)
    assert returnFoo.discriminator == common.fruit.papaya
    assert returnFoo.bing.x == 4
    assert returnFoo.bing.y == float(3)


def test_NestedStructs(client):
    myFoo = common.foo()
    returnFoo = common.foo()
    myFoo.discriminator = common.fruit.apple
    myFoo.bing.a = float(2.0)
    myFoo.bing.myFoobar = common.foobar(a=float(0), rawString=None)
    myFoo.bing.myFoobar.rawString = bytearray(b'')
    for x in range(10):
        myFoo.bing.myFoobar.rawString.append(x + 1)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    returnFoo = client.sendMyFoo(myFoo)
    assert returnFoo.discriminator == common.fruit.returnVal
    assert returnFoo.bing.ret == 0xAA


def test_UnionAnn(client):
    myFoo = common.unionType()
    discriminator = common.fruit()
    discriminator = common.fruit.orange
    myFoo.a = [a + 1 for a in range(5)]
    returnFoo = common.foo()
    # send request to the server
    print('\r\neRPC request is sent to the server')
    returnFoo = client.sendMyUnion(discriminator, myFoo)
    assert returnFoo.discriminator == common.fruit.returnVal
    assert returnFoo.bing.ret == 0xAA

    discriminator = common.fruit.banana
    myFoo.x = 3
    myFoo.y = float(4.0)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    returnFoo = client.sendMyUnion(discriminator, myFoo)
    assert returnFoo.bing.x == 4
    assert returnFoo.bing.y == float(3)


def test_Quit(common_client):
    # send request to the server
    print("eRPC quit request send to the server.")
    return common_client.quit()
