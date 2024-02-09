#!/usr/bin/python

# Copyright 2021 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import inspect, os, sys
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))))
from server.runner import Runner
from service.test_typedef.erpc_outputs import test as test_typedef
from service.test_typedef.erpc_outputs import test_unit_test_common
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
class TypedefTestServiceHandler(test_typedef.interface.ITypedefService):
    def sendReceiveInt(self, a):
        return a * 2 + 1

    def sendReceiveEnum(self, a):
        return a + 1

    def sendReceiveStruct(self, a):
        b = test_typedef.common.B(m=(2 * a.m), n=(2 + a.n))
        return b

    def sendReceiveListType(self, received_list):
        send_list = []
        for y in range(len(received_list)):
            send_list.append(2 * received_list[y])
        return send_list

    def sendReceiveString(self, hello):
        return hello + " World!" 

    def sendReceive2ListType(self, received_list):
        send_list = [[] for a in range(len(received_list))]
        for x in range(len(received_list)):
            for y in range(len(received_list[x])):
                send_list[x].append(2 * received_list[x][y])
        return send_list

###############################################################################
# Client
###############################################################################

## Individual test cases / eRPC calls
def runSendReceiveInt(client):
    a = 10
    b = 2 * a + 1
    # send request to the server
    print('\r\neRPC request is sent to the server')
    assert b == client.sendReceiveInt(a)

def runTestTypedefServiceID(client):
    assert 2 == test_typedef.interface.ITypedefService.SERVICE_ID 
    
def runSendReceiveEnum(client):
    a = test_typedef.common.Colors.green
    # send request to the server
    print('\r\neRPC request is sent to the server')
    assert test_typedef.common.Colors.blue == client.sendReceiveEnum(a)

def runSendReceiveStruct(client):
    a = test_typedef.common.B(m=1, n=4)
    b = test_typedef.common.B(m=(2 * a.m), n=(2 + a.n))
    r = test_typedef.common.B()
    # send request to the server
    print('\r\neRPC request is sent to the server')
    r = client.sendReceiveStruct(a)
    assert r.m == b.m
    assert r.n == b.n

def runSendReceiveListType(client):
    send_list = range(12)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    received_list = client.sendReceiveListType(send_list)
    for x in range(len(received_list)):
        assert (received_list[x] / 2) == send_list[x] 

def runSendReceiveString(client):
    send = "Hello"
    print('\r\neRPC request is sent to the server')
    received = client.sendReceiveString(send)
    assert received == "Hello World!" 

def runSendReceive2ListType(client):
    send_list = [[0] for a in range(10)]
    for a in range(10):
        send_list[a] = range(a+1)
    #print(send_list)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    received_list = client.sendReceive2ListType(send_list)
    #print(received_list)
    for x in range(len(received_list)):
        for y in range(len(received_list[x])):
            assert (received_list[x][y] / 2) == send_list[x][y] 

###############################################################################
# PYTESTS
###############################################################################
class TestTypedef:
    server_cmd = [(os.path.abspath(__file__)).replace(os.sep, '/'), '-s']
    services = [test_typedef.server.TypedefServiceService(TypedefTestServiceHandler()), test_unit_test_common.server.CommonService(CommonTestServiceHandler())]
    clientsConstructors = [test_typedef.client.TypedefServiceClient, test_unit_test_common.client.CommonClient]
    runner = None

    def test_runTesttestPrepareEnv(self):
        TestTypedef.runner = Runner('test_typedef', TestTypedef.server_cmd, TestTypedef.services, TestTypedef.clientsConstructors)

    def test_runTestTypedefServiceID(self):
        self.runner.runTest(test_typedef.interface.ITypedefService.SERVICE_ID , runTestTypedefServiceID)

    def test_runSendReceiveInt(self):
        self.runner.runTest(test_typedef.interface.ITypedefService.SERVICE_ID, runSendReceiveInt)

    def test_runSendReceiveEnum(self):
        self.runner.runTest(test_typedef.interface.ITypedefService.SERVICE_ID, runSendReceiveEnum)

    def test_runSendReceiveStruct(self):
        self.runner.runTest(test_typedef.interface.ITypedefService.SERVICE_ID, runSendReceiveStruct)

    def test_runSendReceiveListType(self):
        self.runner.runTest(test_typedef.interface.ITypedefService.SERVICE_ID, runSendReceiveListType)

    def test_runSendReceiveString(self):
        self.runner.runTest(test_typedef.interface.ITypedefService.SERVICE_ID, runSendReceiveString)
    
    def test_runTesttestQuit(self):
        self.runner.runTest(test_unit_test_common.interface.ICommon.SERVICE_ID, runTesttestQuit)

    def runTests(self):
        self.test_runTesttestPrepareEnv()
        self.test_runTestTypedefServiceID()
        self.test_runSendReceiveInt()
        self.test_runSendReceiveEnum()
        self.test_runSendReceiveStruct()
        self.test_runSendReceiveListType()
        self.test_runSendReceiveString()
        self.test_runTesttestQuit()

# runSendReceive2ListType)

###############################################################################
# Main
###############################################################################

if __name__ == "__main__":
    tester = TestTypedef()
    tester.runTests()
