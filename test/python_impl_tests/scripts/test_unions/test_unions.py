#!/usr/bin/python

# Copyright 2021 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import inspect, os, sys
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))))
from server.runner import Runner
from service.test_unions.erpc_outputs import test as test_unions
from service.test_unions.erpc_outputs import test_unit_test_common
from common.common import CommonTestServiceHandler, runTesttestQuit
try:
    import erpc
except ImportError:
    print("Could not import erpc.\r\nPlease install it first by running \"python setup.py install\" in folder \"erpc/erpc_python/\".")
    sys.exit()

###############################################################################
# Variables
###############################################################################

###############################################################################
# Functions
###############################################################################


###############################################################################
# Server
###############################################################################

## Add service handler
class UnionsTestServiceHandler(test_unions.interface.IArithmeticService):
    def testGenericCallback(self, event):
        newEvent = test_unions.common.gapGenericEvent_t(eventType=test_unions.common.gapGenericEventType_t.gInternalError_c)
        newEvent.eventData.internalError = test_unions.common.gapInternalError_t(errorCode=test_unions.common.bleResult_t.gBleSuccess_c, errorSource=test_unions.common.gapInternalErrorSource_t.gHciCommandStatus_c, hciCommandOpcode=5)
        if event.eventType == test_unions.common.gapGenericEventType_t.gInternalError_c:
            if (event.eventData.internalError.errorCode == test_unions.common.bleResult_t.gBleSuccess_c and
                event.eventData.internalError.errorSource == test_unions.common.gapInternalErrorSource_t.gHciCommandStatus_c and
                event.eventData.internalError.hciCommandOpcode == 5):
                newEvent.eventType = test_unions.common.gapGenericEventType_t.gWhiteListSizeReady_c
                newEvent.eventData.whiteListSize = 0
            else:
                newEvent.eventType = test_unions.common.gapGenericEventType_t.gAdvertisingSetupFailed_c
                newEvent.eventData.whiteListSize = 0
        elif event.eventType == test_unions.common.gapGenericEventType_t.gRandomAddressReady_c:
            newEvent.eventType = test_unions.common.gapGenericEventType_t.gTestCaseReturn_c
            newEvent.eventData.returnCode = 1
            for i in range(test_unions.common.gcBleDeviceAddressSize_c):
                if (event.eventData.aAddress[i] != (0xAA ^ 0xFF)):
                    newEvent.eventData.returnCode = 0
        elif event.eventType == test_unions.common.gapGenericEventType_t.gWhiteListSizeReady_c:
            newEvent.eventType = test_unions.common.gapGenericEventType_t.gTestCaseReturn_c
            if (event.eventData.whiteListSize == 100):
                newEvent.eventData.returnCode = 100
            else:
                newEvent.eventData.returnCode = 0
        elif event.eventType == test_unions.common.gapGenericEventType_t.gPublicAddressRead_c:
            pass
        elif event.eventType == test_unions.common.gapGenericEventType_t.gAdvertisingSetupFailed_c:
            pass
        elif event.eventType == test_unions.common.gapGenericEventType_t.gAdvTxPowerLevelRead_c:
            pass
        return newEvent

    def sendMyFoo(self, f):
        newFoo = test_unions.common.foo()
        if f.discriminator == test_unions.common.fruit.apple:
            newFoo.discriminator = test_unions.common.fruit.returnVal
            newFoo.bing.myFoobar = test_unions.common.foobar(a=float(0), rawString=None)
            newFoo.bing.ret = 0xAA
            for i in range(len(f.bing.myFoobar.rawString)):
                if (f.bing.myFoobar.rawString[i] != (i + 1)):
                    newFoo.bing.ret = 0x55
        elif f.discriminator == test_unions.common.fruit.banana:
            newFoo.discriminator = test_unions.common.fruit.papaya
            if (f.bing.x == 3) and (f.bing.y == float(4.0)):
                newFoo.bing.x = 4
                newFoo.bing.y = 3
            else:
                newFoo.bing.x = 1
                newFoo.bing.y = 1
        elif f.discriminator == test_unions.common.fruit.orange:
            newFoo.discriminator = test_unions.common.fruit.returnVal
            newFoo.bing.ret = 0xAA
            for i in range(len(f.bing.a)):
                if (f.bing.a[i] != (i + 1)):
                    newFoo.bing.ret = 0x55
        return newFoo

    def sendMyUnion(self, discriminator, unionVariable):
        newFoo = test_unions.common.foo()
        if discriminator == test_unions.common.fruit.apple:
            newFoo.discriminator = test_unions.common.fruit.returnVal
            newFoo.bing.myFoobar = test_unions.common.foobar(a=float(0), rawString=None)
            newFoo.bing.ret = 0xAA
            for i in range(len(unionVariable.myFoobar.rawString)):
                if (unionVariable.myFoobar.rawString[i] != (i + 1)):
                    newFoo.bing.ret = 0x55
        elif discriminator == test_unions.common.fruit.banana:
            newFoo.discriminator = test_unions.common.fruit.papaya
            if (unionVariable.x == 3) and (unionVariable.y == float(4.0)):
                newFoo.bing.x = 4
                newFoo.bing.y = 3
            else:
                newFoo.bing.x = 1
                newFoo.bing.y = 1
        elif discriminator == test_unions.common.fruit.orange:
            newFoo.discriminator = test_unions.common.fruit.returnVal
            newFoo.bing.ret = 0xAA
            for i in range(len(unionVariable.a)):
                if (unionVariable.a[i] != (i + 1)):
                    newFoo.bing.ret = 0x55
        return newFoo

## Run server on specified transport layer
def runServer(transport):
    # create test unions eRPC service
    handler = UnionsTestServiceHandler()
    service = test_unions.server.ArithmeticServiceService(handler)

    # run server
    server = erpc.simple_server.SimpleServer(transport, erpc.basic_codec.BasicCodec)
    server.add_service(service)
    print('\r\nWait for client to send a eRPC request')
    sys.stdout.flush()
    server.run()

###############################################################################
# Client
###############################################################################

## Individual test cases / eRPC calls

def runTestUnionsServiceID(client):
    assert 2 == test_unions.interface.IArithmeticService.SERVICE_ID 

def runtestGenericCallback(client):
    event = test_unions.common.gapGenericEvent_t(eventType=test_unions.common.gapGenericEventType_t.gInternalError_c)
    event.eventData.internalError = test_unions.common.gapInternalError_t(errorCode=test_unions.common.bleResult_t.gBleSuccess_c, errorSource=test_unions.common.gapInternalErrorSource_t.gHciCommandStatus_c, hciCommandOpcode=5)
    newEvent = test_unions.common.gapGenericEvent_t()
    #print(event)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    newEvent = client.testGenericCallback(event)
    assert newEvent.eventType == test_unions.common.gapGenericEventType_t.gWhiteListSizeReady_c
    
    event.eventType = test_unions.common.gapGenericEventType_t.gRandomAddressReady_c
    event.eventData.aAddress = [0 for a in range(test_unions.common.gcBleDeviceAddressSize_c)]
    for x in range(test_unions.common.gcBleDeviceAddressSize_c):
        event.eventData.aAddress[x] = (0xAA ^ 0xFF)
    print('\r\neRPC request is sent to the server')
    newEvent = client.testGenericCallback(event)
    assert newEvent.eventType == test_unions.common.gapGenericEventType_t.gTestCaseReturn_c
    assert newEvent.eventData.returnCode == 1

    event.eventType = test_unions.common.gapGenericEventType_t.gWhiteListSizeReady_c
    event.eventData.whiteListSize = 100
    print('\r\neRPC request is sent to the server')
    newEvent = client.testGenericCallback(event)
    assert newEvent.eventType == test_unions.common.gapGenericEventType_t.gTestCaseReturn_c
    assert newEvent.eventData.returnCode == 100

def runtestUnionLists(client):
    myFoo = test_unions.common.foo()
    returnFoo = test_unions.common.foo()
    myFoo.discriminator = test_unions.common.fruit.orange
    myFoo.bing.a = [a+1 for a in range(5)]
    # send request to the server
    print('\r\neRPC request is sent to the server')
    returnFoo = client.sendMyFoo(myFoo)
    assert returnFoo.discriminator == test_unions.common.fruit.returnVal
    assert returnFoo.bing.ret == 0xAA
    
    myFoo.discriminator = test_unions.common.fruit.banana
    myFoo.bing.x = 3
    myFoo.bing.y = float(4.0)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    returnFoo = client.sendMyFoo(myFoo)
    assert returnFoo.discriminator == test_unions.common.fruit.papaya
    assert returnFoo.bing.x == 4
    assert returnFoo.bing.y == float(3)

def runtestNestedStructs(client):
    myFoo = test_unions.common.foo()
    returnFoo = test_unions.common.foo()
    myFoo.discriminator = test_unions.common.fruit.apple
    myFoo.bing.a = float(2.0)
    myFoo.bing.myFoobar = test_unions.common.foobar(a=float(0), rawString=None)
    myFoo.bing.myFoobar.rawString = bytearray(b'')
    for x in range(10):
        myFoo.bing.myFoobar.rawString.append(x+1)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    returnFoo = client.sendMyFoo(myFoo)
    assert returnFoo.discriminator == test_unions.common.fruit.returnVal
    assert returnFoo.bing.ret == 0xAA
    
def runtestUnionAnn(client):
    myFoo = test_unions.common.unionType()
    discriminator = test_unions.common.fruit()
    discriminator = test_unions.common.fruit.orange
    myFoo.a = [a+1 for a in range(5)]
    returnFoo = test_unions.common.foo()
    # send request to the server
    print('\r\neRPC request is sent to the server')
    returnFoo = client.sendMyUnion(discriminator, myFoo)
    assert returnFoo.discriminator == test_unions.common.fruit.returnVal
    assert returnFoo.bing.ret == 0xAA
    
    discriminator = test_unions.common.fruit.banana
    myFoo.x = 3
    myFoo.y = float(4.0)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    returnFoo = client.sendMyUnion(discriminator, myFoo)
    assert returnFoo.bing.x == 4
    assert returnFoo.bing.y == float(3)

###############################################################################
# PYTESTS
###############################################################################
class TestUnions:
    server_cmd = [(os.path.abspath(__file__)).replace(os.sep, '/'), '-s']
    services = [test_unions.server.ArithmeticServiceService(UnionsTestServiceHandler()), test_unit_test_common.server.CommonService(CommonTestServiceHandler())]
    clientsConstructors = [test_unions.client.ArithmeticServiceClient, test_unit_test_common.client.CommonClient]
    runner = None

    def test_runTesttestPrepareEnv(self):
        TestUnions.runner = Runner('test_unions', TestUnions.server_cmd, TestUnions.services, TestUnions.clientsConstructors)

    def test_runTestUnionsServiceID(self):
        self.runner.runTest(test_unions.interface.IArithmeticService.SERVICE_ID , runTestUnionsServiceID)

    def test_runtestGenericCallback(self):
        self.runner.runTest(test_unions.interface.IArithmeticService.SERVICE_ID, runtestGenericCallback)

    def test_runtestUnionLists(self):
        self.runner.runTest(test_unions.interface.IArithmeticService.SERVICE_ID, runtestUnionLists)

    def test_runtestNestedStructs(self):
        self.runner.runTest(test_unions.interface.IArithmeticService.SERVICE_ID, runtestNestedStructs)

    def test_runtestUnionAnn(self):
        self.runner.runTest(test_unions.interface.IArithmeticService.SERVICE_ID, runtestUnionAnn)

    def test_runTesttestQuit(self):
        self.runner.runTest(test_unit_test_common.interface.ICommon.SERVICE_ID, runTesttestQuit)

    def runTests(self):
        self.test_runTesttestPrepareEnv()
        self.test_runTestUnionsServiceID()
        self.test_runtestGenericCallback()
        self.test_runtestUnionLists()
        self.test_runtestNestedStructs()
        self.test_runTesttestQuit()

###############################################################################
# Main
###############################################################################

if __name__ == "__main__":
    tester = TestUnions()
    tester.runTests()
