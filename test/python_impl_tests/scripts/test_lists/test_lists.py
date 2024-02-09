#!/usr/bin/python

# Copyright 2021 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import inspect, os, sys
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))))
from server.runner import Runner
from service.test_lists.erpc_outputs import test as test_lists
from service.test_lists.erpc_outputs import test_unit_test_common
from common.common import CommonTestServiceHandler, runTesttestQuit
try:
    import erpc
except ImportError:
    print("Could not import erpc.\r\nPlease install it first by running \"python setup.py install\" in folder \"erpc/erpc_python/\".")
    sys.exit()
import pytest

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
class ListsTestServiceHandler(test_lists.interface.IPointersService):
    def sendReceivedInt32(self, received_list):
        send_list = []
        for y in range(len(received_list)):
            send_list.append(2 * received_list[y])
        return send_list

    def sendReceived2Int32(self, received_list):
        send_list = [[] for a in range(len(received_list))]
        for x in range(len(received_list)):
            for y in range(len(received_list[x])):
                send_list[x].append(2 * received_list[x][y])
        return send_list

    def sendReceivedEnum(self, received_list):
        send_list = []
        for y in range(len(received_list)):
            send_list.append(received_list[y])
        return send_list

    def sendReceived2Enum(self, received_list):
        send_list = [[] for a in range(len(received_list))]
        for x in range(len(received_list)):
            for y in range(len(received_list[x])):
                send_list[x].append(received_list[x][y])
        return send_list

    def sendReceivedStruct(self, received_list):
        send_list = [test_lists.common.C() for b in range(len(received_list))]
        for y in range(len(received_list)):
            send_list[y].m = 2 * received_list[y].m
            send_list[y].n = 2 * received_list[y].n
        return send_list

    def sendReceived2Struct(self, received_list):
        send_list = [[] for a in range(len(received_list))]
        for x in range(len(received_list)):
            send_list[x] = [test_lists.common.C() for b in range(len(received_list[x]))]
            for y in range(len(received_list[x])):
                send_list[x][y].m = 2 * received_list[x][y].m
                send_list[x][y].n = 2 * received_list[x][y].n
        return send_list

    def sendReceivedString(self, received_list):
        send_list = []
        for y in range(len(received_list)):
            send_list.append(received_list[y] + " World!")
        return send_list

    def sendReceived2String(self, received_list):
        send_list = [[] for a in range(len(received_list))]
        for x in range(len(received_list)):
            for y in range(len(received_list[x])):
                send_list[x].append(received_list[x][y] + " World!")
        return send_list

    @pytest.mark.skip(reason="They do not belong to the tests.")
    def test_list_allDirection(self, a, b, e):
        #e list is defined as an inout parameter 
        for y in range(len(a)):
            e.value[y] = 2 * e.value[y]
        return

    def testLengthAnnotation(self, myList):
        for y in range(len(myList)):
            if myList[y] != y + 1:
                return 0
        return 1

    def testLengthAnnotationInStruct(self, s):
        for y in range(s.len):
            if s.myList[y] != y + 1:
                return 0
        return 10

    def returnSentStructLengthAnnotation(self, s):
        ret = test_lists.common.listStruct([])
        for x in range(s.len):
            ret.myList.append(s.myList[x])
        return ret

    def sendGapAdvertisingData(self, ad):
        for x in range(ad.cNumAdStructures):
            if ad.aAdStructures[x].adType != 5:
                return 0
            for y in range(ad.aAdStructures[x].length):
                if ad.aAdStructures[x].aData[y] != ((y + 1) * 5):
                    return 0
        return 33
          
##################
## Run server on specified transport layer
def runServer(transport):
    # create test lists eRPC service
    handler = ListsTestServiceHandler()
    service = test_lists.server.PointersServiceService(handler)

    # run server
    server = erpc.simple_server.SimpleServer(transport, erpc.basic_codec.BasicCodec)
    server.add_service(service)
    print('\r\nWait for client to send a eRPC request')
    sys.stdout.flush()
    server.run()

##################
###############################################################################
# Client
###############################################################################

## Individual test cases / eRPC calls

def runTestListsServiceID(client):
    assert 2 == test_lists.interface.IPointersService.SERVICE_ID 

def runSendReceivedInt32(client):
    send_list = range(12)
    print('\r\neRPC request is sent to the server')
    received_list = client.sendReceivedInt32(send_list)
    for x in range(len(received_list)):
        assert (received_list[x] / 2) == send_list[x] 

def runSendReceiveZeroSize(client):
    send_list = [0]
    print('\r\neRPC request is sent to the server')
    received_list = client.sendReceivedInt32(send_list)
    assert received_list == [0]

def runSendReceived2Int32(client):
    send_list = [[0] for a in range(10)]
    for a in range(10):
        send_list[a] = range(a+1)
    #print(send_list)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    received_list = client.sendReceived2Int32(send_list)
    #print(received_list)
    for x in range(len(received_list)):
        for y in range(len(received_list[x])):
            assert (received_list[x][y] / 2) == send_list[x][y] 

def runSendReceivedEnum(client):
    send_list = []
    send_list.append(test_lists.common.enumColor.red)
    send_list.append(test_lists.common.enumColor.green)
    send_list.append(test_lists.common.enumColor.blue)
    print('\r\neRPC request is sent to the server')
    received_list = client.sendReceivedEnum(send_list)
    #print(received_list)
    for x in range(len(received_list)):
        assert received_list[x] == send_list[x] 

def runSendReceived2Enum(client):
    send_list = [[] for a in range(3)]
    for a in range(3):
        for b in range(a+1):
            if b == 0:
                send_list[a].append(test_lists.common.enumColor.red)
            if b == 1:
                send_list[a].append(test_lists.common.enumColor.green)
            if b == 2:
                send_list[a].append(test_lists.common.enumColor.blue)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    received_list = client.sendReceived2Enum(send_list)
    #print(received_list)
    for x in range(len(received_list)):
        assert received_list[x] == send_list[x] 

def runSendReceivedStruct(client):
    send_list = []
    for a in range(10):
        send_list.append(test_lists.common.C(a, a * 2))
    #print(send_list)
    print('\r\neRPC request is sent to the server')
    received_list = client.sendReceivedStruct(send_list)
    #print(received_list)
    for x in range(len(received_list)):
        assert (received_list[x].m) / 2 == send_list[x].m 
        assert (received_list[x].n) / 2 == send_list[x].n 

def runSendReceived2Struct(client):
    send_list = [[] for a in range(10)]
    for a in range(10):
        for b in range(a+1):
            send_list[a].append(test_lists.common.C(a, b))
    #print(send_list)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    received_list = client.sendReceived2Struct(send_list)
    #print(received_list)
    for x in range(len(received_list)):
        for y in range(len(received_list[x])):
            assert (received_list[x][y].m) / 2 == send_list[x][y].m 
            assert (received_list[x][y].n) / 2 == send_list[x][y].n 

def runSendReceivedString(client):
    send_list = ["Hello" for a in range(12)]
    #print(send_list)
    print('\r\neRPC request is sent to the server')
    received_list = client.sendReceivedString(send_list)
    #print(received_list)
    for x in range(len(received_list)):
        assert received_list[x] == "Hello World!" 

def runSendReceived2String(client):
    send_list = [[] for a in range(12)]
    for a in range(12):
        for b in range(a+1):
            send_list[a].append("Hello")
    #print(send_list)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    received_list = client.sendReceived2String(send_list)
    #print(received_list)
    for x in range(len(received_list)):
        for y in range(len(received_list[x])):
            assert received_list[x][y] == "Hello World!" 

def runtest_list_allDirection(client):
    send_list_a = []
    send_list_b = []
    send_list_e = erpc.Reference()
    send_list_e.value = []
    for x in range(10):
        send_list_a.append(x)
        send_list_b.append(2 * x)
        send_list_e.value.append(6 * x)
    #print(send_list_e)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    client.test_list_allDirection(send_list_a, send_list_b, send_list_e)
    #print(send_list_e)
    for x in range(len(send_list_a)):
        assert send_list_e.value[x] == 12 * send_list_a[x]

def runtestLengthAnnotation(client):
    length = 5
    list = []
    for x in range(length):
        list.append(x + 1)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    result = client.testLengthAnnotation(list)
    assert result == 1

def runtestLengthAnnotationInStruct(client):
    myListStruct = test_lists.common.listStruct([])
    for x in range(5):
        myListStruct.myList.append(x + 1)
    #print(myListStruct)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    result = client.testLengthAnnotationInStruct(myListStruct)
    assert result == 10

def runreturnSentStructLengthAnnotation(client):
    myListStruct = test_lists.common.listStruct([])
    returnStruct = test_lists.common.listStruct([])
    for x in range(5):
        myListStruct.myList.append(x + 1)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    returnStruct = client.returnSentStructLengthAnnotation(myListStruct)
    for x in range(returnStruct.len):
        assert myListStruct.myList[x] == returnStruct.myList[x] 

def runsendGapAdvertisingData(client):
    ad = test_lists.common.gapAdvertisingData_t([])
    for x in range(2):
        ad.aAdStructures.append(test_lists.common.gapAdStructure_t(adType=5, aData=[((a + 1) * 5) for a in range(3)]))
    #print(ad)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    result = client.sendGapAdvertisingData(ad)
    assert result == 33

###############################################################################
# PYTESTS
###############################################################################
class TestLists:
    server_cmd = [(os.path.abspath(__file__)).replace(os.sep, '/'), '-s']
    services = [test_lists.server.PointersServiceService(ListsTestServiceHandler()), test_unit_test_common.server.CommonService(CommonTestServiceHandler())]
    clientsConstructors = [test_lists.client.PointersServiceClient, test_unit_test_common.client.CommonClient]
    runner = None

    def test_runTesttestPrepareEnv(self):
        TestLists.runner = Runner('test_lists', TestLists.server_cmd, TestLists.services, TestLists.clientsConstructors)

    def test_runTestListsServiceID(self):
        self.runner.runTest(test_lists.interface.IPointersService.SERVICE_ID, runTestListsServiceID)

    def test_runSendReceivedInt32(self):
        self.runner.runTest(test_lists.interface.IPointersService.SERVICE_ID, runSendReceivedInt32)

    def test_runSendReceiveZeroSize(self):
        self.runner.runTest(test_lists.interface.IPointersService.SERVICE_ID, runSendReceiveZeroSize)

    def test_runSendReceived2Int32(self):
        self.runner.runTest(test_lists.interface.IPointersService.SERVICE_ID, runSendReceived2Int32)   #TODO C failed

    def test_runSendReceivedEnum(self):
        self.runner.runTest(test_lists.interface.IPointersService.SERVICE_ID, runSendReceivedEnum)

    def test_runSendReceived2Enum(self):
        self.runner.runTest(test_lists.interface.IPointersService.SERVICE_ID, runSendReceived2Enum)

    def test_runSendReceivedStruct(self):
        self.runner.runTest(test_lists.interface.IPointersService.SERVICE_ID, runSendReceivedStruct)

    def test_runSendReceived2Struct(self):
        self.runner.runTest(test_lists.interface.IPointersService.SERVICE_ID, runSendReceived2Struct)  #TODO C failed

    def test_runSendReceivedString(self):
        self.runner.runTest(test_lists.interface.IPointersService.SERVICE_ID, runSendReceivedString)

    def test_runSendReceived2String(self):
        self.runner.runTest(test_lists.interface.IPointersService.SERVICE_ID, runSendReceived2String)  #TODO C failed

    def test_runtest_list_allDirection(self):
        self.runner.runTest(test_lists.interface.IPointersService.SERVICE_ID, runtest_list_allDirection)

    def test_runtestLengthAnnotation(self):
        self.runner.runTest(test_lists.interface.IPointersService.SERVICE_ID, runtestLengthAnnotation)

    def test_runtestLengthAnnotationInStruct(self):
        self.runner.runTest(test_lists.interface.IPointersService.SERVICE_ID, runtestLengthAnnotationInStruct)

    def test_runreturnSentStructLengthAnnotation(self):
        self.runner.runTest(test_lists.interface.IPointersService.SERVICE_ID, runreturnSentStructLengthAnnotation)

    def test_runsendGapAdvertisingData(self):
        self.runner.runTest(test_lists.interface.IPointersService.SERVICE_ID, runsendGapAdvertisingData)

    def test_runTesttestQuit(self):
        self.runner.runTest(test_unit_test_common.interface.ICommon.SERVICE_ID, runTesttestQuit)

    def runTests(self):
        self.test_runTesttestPrepareEnv()
        self.test_runTestListsServiceID()
        self.test_runSendReceivedInt32()
        self.test_runSendReceiveZeroSize()
        self.test_runSendReceived2Int32()  #TODO C failed
        self.test_runSendReceivedEnum()
        self.test_runSendReceived2Enum()
        self.test_runSendReceivedStruct()
        self.test_runSendReceived2Struct() #TODO C failed
        self.test_runSendReceivedString()
        self.test_runSendReceived2String() #TODO C failed
        self.test_runtest_list_allDirection()
        self.test_runtestLengthAnnotation()
        self.test_runtestLengthAnnotationInStruct()
        self.test_runreturnSentStructLengthAnnotation()
        self.test_runsendGapAdvertisingData()
        self.test_runTesttestQuit()

###############################################################################
# Main
###############################################################################

if __name__ == "__main__":
    tester = TestLists()
    tester.runTests()
