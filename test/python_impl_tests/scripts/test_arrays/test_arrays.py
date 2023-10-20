#!/usr/bin/python

# Copyright 2021 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import inspect, os, sys
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))))
from server.runner import Runner
try:
    import erpc
except ImportError:
    print("Could not import erpc.\r\nPlease install it first by running \"python setup.py install\" in folder \"erpc/erpc_python/\".")
    sys.exit()
from service.test_arrays import test as test_arrays
from service.test_arrays import test_unit_test_common
from common.common import CommonTestServiceHandler, runTesttestQuit

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
class ArraysTestServiceHandler(test_arrays.interface.IPointersService):
    def sendReceivedInt32(self, arrayNumbers):
        print('sendReceivedInt32 service reached\r\n=============')
        sys.stdout.flush()
        sendArrays = []
        for y in range(12):
            #sendArrays[y] = arrayNumbers[y]
            sendArrays.append(arrayNumbers[y])
        return sendArrays

    def sendReceived2Int32(self, arrayNumbers):
        print('sendReceived2Int32 service reached\r\n=============')
        sys.stdout.flush()
        sendArrays = [[0 for a in range(10)] for b in range(12)]
        for y in range(12):
            for z in range(10):
                sendArrays[y][z] = arrayNumbers[y][z]
        return sendArrays

    def sendReceivedString(self, arrayStrings):
        print('sendReceivedString service reached\r\n=============')
        sys.stdout.flush()
        sendArrays = ["" for a in range(12)]
        for y in range(12):
            sendArrays[y] = arrayStrings[y]
        return sendArrays

    def sendReceived2String(self, arrayStrings):
        print('sendReceived2String service reached\r\n=============')
        sys.stdout.flush()
        sendArrays = [["" for a in range(5)] for b in range(3)]
        for y in range(3):
            for z in range(5):
                sendArrays[y][z] = arrayStrings[y][z]
        return sendArrays

    def sendReceivedEnum(self, arrayEnums):
        print('sendReceivedEnum service reached\r\n=============')
        sys.stdout.flush()
        sendArrays = []
        for y in range(3):
            sendArrays.append(arrayEnums[y])
        return sendArrays

    def sendReceived2Enum(self, arrayEnums):
        print('sendReceived2Enum service reached\r\n=============')
        sys.stdout.flush()
        sendArrays = [[0 for a in range(3)] for b in range(3)]
        for y in range(3):
            for z in range(3):
                sendArrays[y][z] = arrayEnums[y][z]
        return sendArrays

    def sendReceivedList(self, arrayLists):
        print('sendReceivedList service reached\r\n=============')
        sys.stdout.flush()
        sendArrays = []
        for y in range(2):
            sendArrays.append(arrayLists[y])
        return sendArrays

    def sendReceived2List(self, arrayLists):
        print('ssendReceived2List service reached\r\n=============')
        sys.stdout.flush()
        sendArrays = [[0 for a in range(2)] for b in range(2)]
        for y in range(2):
            for z in range(2):
                sendArrays[y][z] = arrayLists[y][z]
        return sendArrays

    def sendReceivedInt32Type(self, arrayNumbers):
        print('sendReceivedInt32Type service reached\r\n=============')
        sys.stdout.flush()
        sendArrays = []
        for y in range(12):
            #sendArrays[y] = arrayNumbers[y]
            sendArrays.append(arrayNumbers[y])
        return sendArrays

    def sendReceived2Int32Type(self, arrayNumbers):
        print('sendReceived2Int32Type service reached\r\n=============')
        sys.stdout.flush()
        sendArrays = []
        for y in range(12):
            #sendArrays[y] = arrayNumbers[y]
            sendArrays.append(arrayNumbers[y])
        return sendArrays

    def sendReceivedStringType(self, arrayStrings):
        print('sendReceivedStringType service reached\r\n=============')
        sys.stdout.flush()
        sendArrays = ["" for a in range(12)]
        for y in range(12):
            sendArrays[y] = arrayStrings[y]
        return sendArrays

    def sendReceived2StringType(self, arrayStrings):
        print('sendReceived2StringType service reached\r\n=============')
        sys.stdout.flush()
        sendArrays = [["" for a in range(5)] for b in range(3)]
        for y in range(3):
            for z in range(5):
                sendArrays[y][z] = arrayStrings[y][z]
        return sendArrays

    def sendReceivedEnumType(self, arrayEnums):
        print('sendReceivedEnumType service reached\r\n=============')
        sys.stdout.flush()
        sendArrays = []
        for y in range(3):
            sendArrays.append(arrayEnums[y])
        return sendArrays

    def sendReceived2EnumType(self, arrayEnums):
        print('sendReceived2EnumType service reached\r\n=============')
        sys.stdout.flush()
        sendArrays = [[0 for a in range(3)] for b in range(3)]
        for y in range(3):
            for z in range(3):
                sendArrays[y][z] = arrayEnums[y][z]
        return sendArrays

    def sendReceivedStructType(self, arrayStructs):
        print('sendReceivedStructType service reached\r\n=============')
        #print(arrayStructs)
        sys.stdout.flush()
        sendArrays = [test_arrays.common.C() for b in range(3)]
        for y in range(3):
            sendArrays[y].m = arrayStructs[y].m
            sendArrays[y].n = arrayStructs[y].n
        return sendArrays

    def sendReceived2StructType(self, arrayStructs):
        print('sendReceived2StructType service reached\r\n=============')
        #print(arrayStructs)
        sys.stdout.flush()
        sendArrays = [[test_arrays.common.C() for a in range(3)] for b in range(3)]
        for y in range(3):
            for z in range(3):
                sendArrays[y][z].m = arrayStructs[y][z].m
                sendArrays[y][z].n = arrayStructs[y][z].n
        return sendArrays

    def sendReceivedListType(self, arrayLists):
        print('sendReceivedListType service reached\r\n=============')
        sys.stdout.flush()
        sendArrays = []
        for y in range(2):
            sendArrays.append(arrayLists[y])
        return sendArrays

    def sendReceived2ListType(self, arrayLists):
        print('sendReceived2ListType service reached\r\n=============')
        sys.stdout.flush()
        sendArrays = [[0 for a in range(2)] for b in range(2)]
        for y in range(2):
            for z in range(2):
                sendArrays[y][z] = arrayLists[y][z]
        return sendArrays

    def sendReceiveStruct(self, all_types):
        print('sendReceiveStruct service reached\r\n=============')
        #print(all_types)
        sys.stdout.flush()
        sendArrays = [test_arrays.common.AllTypes() for b in range(2)]
        for y in range(2):
            sendArrays[y].c = test_arrays.common.C()
            sendArrays[y].number = all_types[y].number
            sendArrays[y].text = all_types[y].text
            sendArrays[y].color = all_types[y].color
            sendArrays[y].c.m = all_types[y].c.m
            sendArrays[y].c.n = all_types[y].c.n
            sendArrays[y].list_numbers = all_types[y].list_numbers
            sendArrays[y].list_text = all_types[y].list_text
            sendArrays[y].array_numbers = all_types[y].array_numbers
            sendArrays[y].array_text = all_types[y].array_text
        return sendArrays

    def sendReceive2Struct(self, all_types):
        print('sendReceive2Struct service reached\r\n=============')
        #print(all_types)
        sys.stdout.flush()
        sendArrays = [[test_arrays.common.AllTypes() for a in range(1)] for b in range(1)]
        for y in range(1):
            for z in range(1):
                sendArrays[y][z].c = test_arrays.common.C()
                sendArrays[y][z].number = all_types[y][z].number
                sendArrays[y][z].text = all_types[y][z].text
                sendArrays[y][z].color = all_types[y][z].color
                sendArrays[y][z].c.m = all_types[y][z].c.m
                sendArrays[y][z].c.n = all_types[y][z].c.n
                sendArrays[y][z].list_numbers = all_types[y][z].list_numbers
                sendArrays[y][z].list_text = all_types[y][z].list_text
                sendArrays[y][z].array_numbers = all_types[y][z].array_numbers
                sendArrays[y][z].array_text = all_types[y][z].array_text
        #print(sendArrays)
        return sendArrays

    def test_array_allDirection(self, a, b, c, d):
        print('test_array_allDirection service reached\r\n=============')
        sys.stdout.flush()
        #c array is defined as an out parameter and needs to be initialized first 
        c.value = [0 for a in range(5)]
        for y in range(5):
            c.value[y] = y + 1
            d.value[y] = y * 8
        return

###############################################################################
# Client
###############################################################################

## Individual test cases / eRPC calls
def runTestsendReceivedInt32(client):
    send_array = range(12)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    received_array = client.sendReceivedInt32(send_array)
    for x in range(12):
        assert received_array[x] == send_array[x] 
        #print(received_array[x])

def runTestArraysServiceID(client):
    assert 2 == test_arrays.interface.IPointersService.SERVICE_ID 

def runTestsendReceived2Int32(client):
    send_array = [[0 for a in range(10)] for b in range(12)]
    #print(send_array)
    for a in range(12):
        for b in range(10):
            send_array[a][b] = a*b
    #print(send_array)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    received_array = client.sendReceived2Int32(send_array)
    for a in range(12):
        for b in range(10):
            assert received_array[a][b] == send_array[a][b]

def runTestsendReceivedString(client):
    send_array = ["Hello" for a in range(12)]
    #print(send_array)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    received_array = client.sendReceivedString(send_array)
    for x in range(12):
        assert received_array[x] == send_array[x] 

def runTestsendReceived2String(client):
    send_array = [["Hello" for a in range(5)] for b in range(3)]
    received_array = [["" for a in range(5)] for b in range(3)]
    #print(send_array)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    received_array = client.sendReceived2String(send_array)
    for a in range(3):
        for b in range(5):
            assert received_array[a][b] == send_array[a][b] 
    
def runTestsendReceivedEnum(client):
    send_array = []
    send_array.append(test_arrays.common.enumColor.red)
    send_array.append(test_arrays.common.enumColor.green)
    send_array.append(test_arrays.common.enumColor.blue)
    #print(send_array)
    print('\r\neRPC request is sent to the server')
    received_array = client.sendReceivedEnum(send_array)
    for x in range(3):
        assert received_array[x] == send_array[x] 

def runTestsendReceived2Enum(client):
    send_array = [[0 for a in range(3)] for b in range(3)]
    for a in range(3):
        send_array[a][0] = test_arrays.common.enumColor.red
        send_array[a][1] = test_arrays.common.enumColor.green
        send_array[a][2] = test_arrays.common.enumColor.blue
    #print(send_array)
    print('\r\neRPC request is sent to the server')
    received_array = client.sendReceived2Enum(send_array)
    for a in range(3):
        for b in range(3):
            assert received_array[a][b] == send_array[a][b]

def runTestsendReceivedList(client):
    send_array = [[0, 1] for a in range(2)]
    #print(send_array)
    print('\r\neRPC request is sent to the server')
    received_array = client.sendReceivedList(send_array)
    for x in range(2):
        assert received_array[x] == send_array[x] 

def runTestsendReceived2List(client):
    send_array = [[[0, 1] for a in range(2)] for b in range(2)]
    received_array = [[0 for a in range(2)] for b in range(2)]
    #print(send_array)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    received_array = client.sendReceived2List(send_array)
    for a in range(2):
        for b in range(2):
            assert received_array[a][b] == send_array[a][b] 
    
def runTestsendReceivedInt32Type(client):
    send_array = range(12)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    received_array = client.sendReceivedInt32Type(send_array)
    for x in range(12):
        assert received_array[x] == send_array[x] 
        #print(received_array[x])

def runTestsendReceived2Int32Type(client):
    send_array = [[0 for a in range(10)] for b in range(12)]
    #print(send_array)
    for a in range(12):
        for b in range(10):
            send_array[a][b] = a*b
    #print(send_array)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    received_array = client.sendReceived2Int32Type(send_array)
    for a in range(12):
        for b in range(10):
            assert received_array[a][b] == send_array[a][b]

def runTestsendReceivedStringType(client):
    send_array = ["Hello" for a in range(12)]
    #print(send_array)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    received_array = client.sendReceivedStringType(send_array)
    for x in range(12):
        assert received_array[x] == send_array[x] 

def runTestsendReceived2StringType(client):
    send_array = [["Hello" for a in range(5)] for b in range(3)]
    received_array = [["" for a in range(5)] for b in range(3)]
    #print(send_array)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    received_array = client.sendReceived2StringType(send_array)
    for a in range(3):
        for b in range(5):
            assert received_array[a][b] == send_array[a][b] 
    
def runTestsendReceivedEnumType(client):
    send_array = []
    send_array.append(test_arrays.common.enumColor.red)
    send_array.append(test_arrays.common.enumColor.green)
    send_array.append(test_arrays.common.enumColor.blue)
    #print(send_array)
    print('\r\neRPC request is sent to the server')
    received_array = client.sendReceivedEnumType(send_array)
    for x in range(3):
        assert received_array[x] == send_array[x] 

def runTestsendReceived2EnumType(client):
    send_array = [[0 for a in range(3)] for b in range(3)]
    for a in range(3):
        send_array[a][0] = test_arrays.common.enumColor.red
        send_array[a][1] = test_arrays.common.enumColor.green
        send_array[a][2] = test_arrays.common.enumColor.blue
    #print(send_array)
    print('\r\neRPC request is sent to the server')
    received_array = client.sendReceived2EnumType(send_array)
    for a in range(3):
        for b in range(3):
            assert received_array[a][b] == send_array[a][b]

def runTestsendReceivedStructType(client):
    send_array = [0 for b in range(3)]
    for a in range(3):
        send_array[a] = test_arrays.common.C(a, a + 2)
    #print(send_array)
    print('\r\neRPC request is sent to the server')
    received_array = client.sendReceivedStructType(send_array)
    #print(received_array)
    for x in range(3):
        assert received_array[x].m == send_array[x].m 
        assert received_array[x].n == send_array[x].n 

def runTestsendReceived2StructType(client):
    send_array = [[0 for a in range(3)] for b in range(3)]
    for a in range(3):
        for b in range(3):
            send_array[a][b] = test_arrays.common.C(a, b)
    #print(send_array)
    print('\r\neRPC request is sent to the server')
    received_array = client.sendReceived2StructType(send_array)
    #print(received_array)
    for a in range(3):
        for b in range(3):
            assert received_array[a][b].m == send_array[a][b].m 
            assert received_array[a][b].n == send_array[a][b].n 

def runTestsendReceivedListType(client):
    send_array = [[0, 1] for a in range(2)]
    #print(send_array)
    print('\r\neRPC request is sent to the server')
    received_array = client.sendReceivedListType(send_array)
    for x in range(2):
        assert received_array[x] == send_array[x] 

def runTestsendReceived2ListType(client):
    send_array = [[[0, 1] for a in range(2)] for b in range(2)]
    received_array = [[0 for a in range(2)] for b in range(2)]
    #print(send_array)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    received_array = client.sendReceived2ListType(send_array)
    for a in range(2):
        for b in range(2):
            assert received_array[a][b] == send_array[a][b] 
    
def runTestsendReceiveStruct(client):
    send_array = [0 for b in range(2)]
    for a in range(2):
        send_array[a] = test_arrays.common.AllTypes(5, "Hello", test_arrays.common.C(5, 20), test_arrays.common.enumColor.red, [0, 1, 2, 3, 4], ["Hello" for b in range(5)], [0, 1, 2, 3, 4], ["Hello" for b in range(5)])
    #print(send_array)
    print('\r\neRPC request is sent to the server')
    received_array = client.sendReceiveStruct(send_array)
    #print(received_array)
    for x in range(2):
        assert received_array[x].number == send_array[x].number
        assert received_array[x].text == send_array[x].text
        assert received_array[x].color == send_array[x].color
        assert received_array[x].c.m == send_array[x].c.m
        assert received_array[x].c.n == send_array[x].c.n
        assert received_array[x].list_numbers == send_array[x].list_numbers
        assert received_array[x].list_text == send_array[x].list_text
        assert received_array[x].array_numbers == send_array[x].array_numbers
        assert received_array[x].array_text == send_array[x].array_text

def runTestsendReceive2Struct(client):
    send_array = [[(test_arrays.common.AllTypes(5, "Hello", test_arrays.common.C(5, 20), test_arrays.common.enumColor.red, [0, 1, 2, 3, 4], ["Hello" for b in range(5)], [0, 1, 2, 3, 4], ["Hello" for b in range(5)])) for x in range(1)] for y in range(1)]
    received_array = [[0 for a in range(1)] for b in range(1)]
    #print(send_array)
    print('\r\neRPC request is sent to the server')
    received_array = client.sendReceive2Struct(send_array)
    #print(received_array)
    for x in range(1):
        for y in range(1):
            assert received_array[x][y].number == send_array[x][y].number
            assert received_array[x][y].text == send_array[x][y].text
            assert received_array[x][y].color == send_array[x][y].color
            assert received_array[x][y].c.m == send_array[x][y].c.m
            assert received_array[x][y].c.n == send_array[x][y].c.n
            assert received_array[x][y].list_numbers == send_array[x][y].list_numbers
            assert received_array[x][y].list_text == send_array[x][y].list_text
            assert received_array[x][y].array_numbers == send_array[x][y].array_numbers
            assert received_array[x][y].array_text == send_array[x][y].array_text

def runTesttest_array_allDirection(client):
    a_array = [0 for a in range(5)]
    b_array = [0 for a in range(5)]
    c_array = erpc.Reference()
    d_array = erpc.Reference()
    c_array.value = [0 for a in range(5)]
    d_array.value = [0 for a in range(5)]
    pA_array = [0 for a in range(5)]
    pB_array = [0 for a in range(5)]
    pC_array = [0 for a in range(5)]
    pD_array = [0 for a in range(5)]
    for a in range(5):
        a_array[a] = a
        b_array[a] = a * 2
        d_array.value[a] = a * 4
        pA_array[a] = a_array[a]
        pB_array[a] = b_array[a]
        pC_array[a] = a + 1
        pD_array[a] = 2 * d_array.value[a]
    # send request to the server
    print('\r\neRPC request is sent to the server')
    client.test_array_allDirection(a_array, b_array, c_array, d_array)
    for a in range(5):
        assert a_array[a] == pA_array[a] 
        assert b_array[a] == pB_array[a] 
        assert c_array.value[a] == pC_array[a] 
        assert d_array.value[a] == pD_array[a] 

###############################################################################
# PYTESTS
###############################################################################
class TestArrays:
    server_cmd = [(os.path.abspath(__file__)).replace(os.sep, '/'), '-s']
    services = [test_arrays.server.PointersServiceService(ArraysTestServiceHandler()), test_unit_test_common.server.CommonService(CommonTestServiceHandler())]
    clientsConstructors = [test_arrays.client.PointersServiceClient, test_unit_test_common.client.CommonClient]
    runner = None

    def test_runTesttestPrepareEnv(self):
        TestArrays.runner = Runner('test_arrays', TestArrays.server_cmd, TestArrays.services, TestArrays.clientsConstructors)

    def test_runTestArraysServiceID(self):
        self.runner.runTest(test_arrays.interface.IPointersService.SERVICE_ID, runTestArraysServiceID)

    def test_runTestsendReceivedInt32(self):
        self.runner.runTest(test_arrays.interface.IPointersService.SERVICE_ID, runTestsendReceivedInt32)

    def test_runTestsendReceived2Int32(self):
        self.runner.runTest(test_arrays.interface.IPointersService.SERVICE_ID, runTestsendReceived2Int32) 

    def test_runTestsendReceivedString(self):
        self.runner.runTest(test_arrays.interface.IPointersService.SERVICE_ID, runTestsendReceivedString)

    def test_runTestsendReceived2String(self):
        self.runner.runTest(test_arrays.interface.IPointersService.SERVICE_ID, runTestsendReceived2String)

    def test_runTestsendReceivedEnum(self):
        self.runner.runTest(test_arrays.interface.IPointersService.SERVICE_ID, runTestsendReceivedEnum)

    def test_runTestsendReceived2Enum(self):
        self.runner.runTest(test_arrays.interface.IPointersService.SERVICE_ID, runTestsendReceived2Enum)

    def test_runTestsendReceivedList(self):
        self.runner.runTest(test_arrays.interface.IPointersService.SERVICE_ID, runTestsendReceivedList)

    def test_runTestsendReceived2List(self):
        self.runner.runTest(test_arrays.interface.IPointersService.SERVICE_ID, runTestsendReceived2List)

    def test_runTestsendReceivedInt32Type(self):
        self.runner.runTest(test_arrays.interface.IPointersService.SERVICE_ID, runTestsendReceivedInt32Type)

    def test_runTestsendReceived2Int32Type(self):
        self.runner.runTest(test_arrays.interface.IPointersService.SERVICE_ID, runTestsendReceived2Int32Type) 

    def test_runTestsendReceivedStringType(self):
        self.runner.runTest(test_arrays.interface.IPointersService.SERVICE_ID, runTestsendReceivedStringType)

    def test_runTestsendReceived2StringType(self):
        self.runner.runTest(test_arrays.interface.IPointersService.SERVICE_ID, runTestsendReceived2StringType)

    def test_runTestsendReceivedStructType(self):
        self.runner.runTest(test_arrays.interface.IPointersService.SERVICE_ID, runTestsendReceivedStructType)

    def test_runTestsendReceived2StructType(self):
        self.runner.runTest(test_arrays.interface.IPointersService.SERVICE_ID, runTestsendReceived2StructType)

    def test_runTestsendReceivedListType(self):
        self.runner.runTest(test_arrays.interface.IPointersService.SERVICE_ID, runTestsendReceivedListType)

    def test_runTestsendReceived2ListType(self):
        self.runner.runTest(test_arrays.interface.IPointersService.SERVICE_ID, runTestsendReceived2ListType)

    def test_runTestsendReceiveStruct(self):
        self.runner.runTest(test_arrays.interface.IPointersService.SERVICE_ID, runTestsendReceiveStruct)   

    def test_runTestsendReceive2Struct(self):
        self.runner.runTest(test_arrays.interface.IPointersService.SERVICE_ID, runTestsendReceive2Struct)

    def test_runTesttest_array_allDirection(self):
        self.runner.runTest(test_arrays.interface.IPointersService.SERVICE_ID, runTesttest_array_allDirection)

    def test_runTesttestQuit(self):
        self.runner.runTest(test_unit_test_common.interface.ICommon.SERVICE_ID, runTesttestQuit)

    def runTests(self):
        self.test_runTesttestPrepareEnv()
        self.test_runTestArraysServiceID()
        self.test_runTestsendReceivedInt32()
        self.test_runTestsendReceived2Int32()
        self.test_runTestsendReceivedString()
        self.test_runTestsendReceived2String()
        self.test_runTestsendReceivedEnum()
        self.test_runTestsendReceived2Enum()
        self.test_runTestsendReceivedList()
        self.test_runTestsendReceived2List()
        self.test_runTestsendReceivedInt32Type()
        self.test_runTestsendReceived2Int32Type() 
        self.test_runTestsendReceivedStringType()
        self.test_runTestsendReceived2StringType()
        self.test_runTestsendReceivedStructType()
        self.test_runTestsendReceived2StructType()
        self.test_runTestsendReceivedListType()
        self.test_runTestsendReceived2ListType()
        self.test_runTestsendReceiveStruct()   
        self.test_runTestsendReceive2Struct()
        self.test_runTesttest_array_allDirection()
        self.test_runTesttestQuit()

###############################################################################
# Main
###############################################################################

if __name__ == "__main__":
    tester = TestArrays()
    tester.runTests()
