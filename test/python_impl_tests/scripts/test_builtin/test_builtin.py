#!/usr/bin/python

# Copyright 2021 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import inspect, os, sys
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))))
from server.runner import Runner
from service.test_builtin.erpc_outputs import test as test_builtin
from service.test_builtin.erpc_outputs import test_unit_test_common
from common.common import CommonTestServiceHandler, runTesttestQuit
try:
    import erpc
except ImportError:
    print("Could not import erpc.\r\nPlease install it first by running \"python setup.py install\" in folder \"erpc/erpc_python/\".")
    sys.exit()

###############################################################################
# Variables
###############################################################################
gInt32AClient = 2
gInt32BClient = -20
gStringAClient = "Hello"
gStringBClient = "World!"
gSaveVariablesServer = {'int32A': 0, 'int32B': 0, 'stringA': "", 'stringB': ""}

###############################################################################
# Functions
###############################################################################


###############################################################################
# Server
###############################################################################

## Add service handler
class BuiltinTestServiceHandler(test_builtin.interface.IBuiltinServices):
    def test_int32_in(self, a):
        global gSaveVariablesServer
        gSaveVariablesServer['int32A'] = a
        return

    def test_int32_in2(self, b):
        global gSaveVariablesServer
        gSaveVariablesServer['int32B'] = b
        return

    def test_int32_out(self, c):
        global gSaveVariablesServer
        #c integer is defined as an out parameter 
        c.value = gSaveVariablesServer['int32A']
        return

    def test_int32_inout(self, e):
        #e integer is defined as an out parameter 
        e.value = e.value + 1
        return

    def test_int32_return(self):
        global gSaveVariablesServer
        return (gSaveVariablesServer['int32A'] * gSaveVariablesServer['int32B'])

    def test_int32_allDirection(self, a, b, c, e):
        c.value = a
        e.value = 2 * e.value
        return a * b

    def test_float_inout(self, a, b):
        #b float is defined as an out parameter and needs to be initialized first 
        b.value = 0.0
        b.value = a
        return

    def test_double_inout(self, a, b):
        #b double is defined as an out parameter and needs to be initialized first 
        b.value = 0.0
        b.value = a
        return

    def test_string_in(self, a):
        global gSaveVariablesServer
        gSaveVariablesServer['stringA'] = a
        return

    def test_string_in2(self, b):
        global gSaveVariablesServer
        gSaveVariablesServer['stringB'] = b
        return

    def test_string_out(self, c):
        global gSaveVariablesServer
        #c string is defined as an out parameter 
        c.value = gSaveVariablesServer['stringA']
        return

    def test_string_inout(self, e):
        global gSaveVariablesServer
        #e string is defined as an inout parameter 
        e.value += gSaveVariablesServer['stringB']
        return

    def test_string_return(self):
        global gSaveVariablesServer
        return (gSaveVariablesServer['stringA'] + " " + gSaveVariablesServer['stringB'])

    def test_string_allDirection(self, a, b, c, e):
        c.value = a
        e.value += gSaveVariablesServer['stringB']
        return e.value

    def sendHello(self, myStr):
        if myStr == "Hello World!":
            return 0
        else:
            return -1

    def sendTwoStrings(self, myStr1, myStr2):
        if (myStr1 == "String one.") and (myStr2 == "String two."):
            return 0
        else:
            return -1

    def returnHello(self):
        return "Hello"

###############################################################################
# Client
###############################################################################

## Individual test cases / eRPC calls
def runTesttest_int32_in_out(client):
    global gInt32AClient
    int32C = erpc.Reference()
    int32C.value = 0
    # send request to the server
    print('\r\neRPC request is sent to the server')
    client.test_int32_in(gInt32AClient)
    print('\r\neRPC request is sent to the server')
    client.test_int32_out(int32C)
    assert gInt32AClient == int32C.value 

def runTestBuiltinServiceID(client):
    assert 2 == test_builtin.interface.IBuiltinServices.SERVICE_ID 

def runTesttest_int32_inout(client):
    int32E = erpc.Reference()
    int32E.value = -6
    for a in range(-5,5):
        # send request to the server
        print('\r\neRPC request is sent to the server')
        client.test_int32_inout(int32E)
        assert a == int32E.value

def runTestest_int32_return(client):
    global gInt32AClient
    global gInt32BClient
    int32R = 0
    # send request to the server
    print('\r\neRPC request is sent to the server')
    client.test_int32_in(gInt32AClient)
    print('\r\neRPC request is sent to the server')
    client.test_int32_in2(gInt32BClient)
    print('\r\neRPC request is sent to the server')
    int32R = client.test_int32_return()
    assert (gInt32AClient * gInt32BClient) == int32R 

def runTestest_int32_allDirection(client):
    global gInt32AClient
    global gInt32BClient
    int32C = erpc.Reference()
    int32C.value = 0
    int32E = erpc.Reference()
    int32E.value = 7
    int32R = 0
    # send request to the server
    print('\r\neRPC request is sent to the server')
    int32R = client.test_int32_allDirection(gInt32AClient, gInt32BClient, int32C, int32E)
    assert int32C.value == gInt32AClient 
    assert int32E.value == 14 
    assert (gInt32AClient * gInt32BClient) == int32R 

def runTestest_float_inout(client):
    a = float(3.14)
    b = erpc.Reference(0.00)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    client.test_float_inout(a, b)
    assert b.value == a 

def runTestest_double_inout(client):
    a = float(3.14)
    b = erpc.Reference(0.00)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    client.test_double_inout(a, b)
    assert b.value == a 

def runTesttest_string_in_out(client):
    global gStringAClient
    stringC = erpc.Reference()
    stringC.value = ""
    # send request to the server
    print('\r\neRPC request is sent to the server')
    client.test_string_in(gStringAClient)
    print('\r\neRPC request is sent to the server')
    client.test_string_out(stringC)
    assert gStringAClient == stringC.value

def runTesttest_string_inout(client):
    stringE = erpc.Reference()
    stringE.value = gStringAClient + " "
    # send request to the server
    print('\r\neRPC request is sent to the server')
    client.test_string_in2(gStringBClient)
    print('\r\neRPC request is sent to the server')
    client.test_string_inout(stringE)
    assert "Hello World!" == stringE.value 

def runTestest_string_return(client):
    stringR = ""
    # send request to the server
    print('\r\neRPC request is sent to the server')
    stringR = client.test_string_return()
    assert "Hello World!" == stringR 

def runTestest_string_allDirection(client):
    global gStringAClient
    global gStringBClient
    stringC = erpc.Reference()
    stringC.value = ""
    stringE = erpc.Reference()
    stringE.value = gStringAClient + " "
    stringR = ""
    # send request to the server
    print('\r\neRPC request is sent to the server')
    stringR = client.test_string_allDirection(gStringAClient, gStringBClient, stringC, stringE)
    assert stringC.value == gStringAClient 
    assert stringE.value == "Hello World!" 
    assert stringR == "Hello World!" 

def runTesStringParamTest1(client):
    int32R = -1
    # send request to the server
    print('\r\neRPC request is sent to the server')
    int32R = client.sendHello("Hello World!")
    assert int32R == 0 

def runTesStringParamTest2(client):
    int32R = -1
    # send request to the server
    print('\r\neRPC request is sent to the server')
    int32R = client.sendTwoStrings("String one.", "String two.")
    assert int32R == 0 

def runTesStringReturnTest1(client):
    # send request to the server
    print('\r\neRPC request is sent to the server')
    assert "Hello" == client.returnHello() 

###############################################################################
# PYTESTS
###############################################################################
class TestBuiltin:
    server_cmd = [(os.path.abspath(__file__)).replace(os.sep, '/'), '-s']
    services = [test_builtin.server.BuiltinServicesService(BuiltinTestServiceHandler()), test_unit_test_common.server.CommonService(CommonTestServiceHandler())]
    clientsConstructors = [test_builtin.client.BuiltinServicesClient, test_unit_test_common.client.CommonClient]
    runner = None

    def test_runTesttestPrepareEnv(self):
        TestBuiltin.runner = Runner('test_builtin', TestBuiltin.server_cmd, TestBuiltin.services, TestBuiltin.clientsConstructors)

    def test_runTestBuiltinServiceID(self):
        self.runner.runTest(test_builtin.interface.IBuiltinServices.SERVICE_ID, runTestBuiltinServiceID)

    def test_runTesttest_int32_in_out(self):
        self.runner.runTest(test_builtin.interface.IBuiltinServices.SERVICE_ID, runTesttest_int32_in_out)

    def test_runTesttest_int32_inout(self):
        self.runner.runTest(test_builtin.interface.IBuiltinServices.SERVICE_ID, runTesttest_int32_inout)

    def test_runTestest_int32_return(self):
        self.runner.runTest(test_builtin.interface.IBuiltinServices.SERVICE_ID, runTestest_int32_return)

    def test_runTestest_int32_allDirection(self):
        self.runner.runTest(test_builtin.interface.IBuiltinServices.SERVICE_ID, runTestest_int32_allDirection)

    def test_runTestest_float_inout(self):
        self.runner.runTest(runTestest_float_inout)

    def test_runTestest_double_inout(self):
        self.runner.runTest(test_builtin.interface.IBuiltinServices.SERVICE_ID, runTestest_double_inout)

    def test_runTesttest_string_in_out(self):
        self.runner.runTest(test_builtin.interface.IBuiltinServices.SERVICE_ID, runTesttest_string_in_out)

    def test_runTesttest_string_inout(self):
        self.runner.runTest(test_builtin.interface.IBuiltinServices.SERVICE_ID, runTesttest_string_inout)

    def test_runTestest_string_return(self):
        self.runner.runTest(test_builtin.interface.IBuiltinServices.SERVICE_ID, runTestest_string_return)

    def test_runTestest_string_allDirection(self):
        self.runner.runTest(test_builtin.interface.IBuiltinServices.SERVICE_ID, runTestest_string_allDirection)

    def test_runTesStringParamTest1(self):
        self.runner.runTest(test_builtin.interface.IBuiltinServices.SERVICE_ID, runTesStringParamTest1)

    def test_runTesStringParamTest2(self):
        self.runner.runTest(test_builtin.interface.IBuiltinServices.SERVICE_ID, runTesStringParamTest2)

    def test_runTesStringReturnTest1(self):
        self.runner.runTest(test_builtin.interface.IBuiltinServices.SERVICE_ID, runTesStringReturnTest1)

    def test_runTesttestQuit(self):
        self.runner.runTest(test_unit_test_common.interface.ICommon.SERVICE_ID, runTesttestQuit)
    
    def runTests(self):
        self.test_runTesttestPrepareEnv()
        self.test_runTestBuiltinServiceID()
        self.test_runTesttest_int32_in_out()
        self.test_runTesttest_int32_inout()
        self.test_runTestest_int32_return()
        self.test_runTestest_int32_allDirection()
        self.test_runTestest_float_inout()
        self.test_runTestest_double_inout()
        self.test_runTesttest_string_in_out()
        self.test_runTesttest_string_inout()
        self.test_runTestest_string_return()
        self.test_runTestest_string_allDirection()
        self.test_runTesStringParamTest1()
        self.test_runTesStringParamTest2()
        self.test_runTesStringReturnTest1()
        self.test_runTesttestQuit()

###############################################################################
# Main
###############################################################################

if __name__ == "__main__":
    tester = TestBuiltin()
    tester.runTests()
