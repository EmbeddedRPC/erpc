#!/usr/bin/python

# Copyright 2021 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import inspect, os, sys
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))))
from server.runner import Runner
from service.test_enums.erpc_outputs import test as test_enums
from service.test_enums.erpc_outputs import test_unit_test_common
from common.common import CommonTestServiceHandler, runTesttestQuit
try:
    import erpc
except ImportError:
    print("Could not import erpc.\r\nPlease install it first by running \"python setup.py install\" in folder \"erpc/erpc_python/\".")
    sys.exit()

###############################################################################
# Variables
###############################################################################
gEnumColorAClient = test_enums.common.enumColor.green
gEnumColorBClient = test_enums.common.enumColor.red
gEnumColorAServer = 0
gEnumColorBServer = 0

###############################################################################
# Functions
###############################################################################


###############################################################################
# Server
###############################################################################

## Add service handler
class EnumsTestServiceHandler(test_enums.interface.IEnumsService):
    def test_enumColor_in(self, a):
        global gEnumColorAServer
        gEnumColorAServer = a
        return

    def test_enumColor_in2(self, b):
        global gEnumColorBServer
        gEnumColorBServer = b
        return

    def test_enumColor_out(self, c):
        #c enum is defined as an out parameter 
        c.value = gEnumColorAServer
        return

    def test_enumColor_inout(self, e):
        #e enum is defined as an out parameter 
        e.value = e.value - 9
        return

    def test_enumColor_return(self):
        return (gEnumColorAServer + 9)

    def test_enumColor_allDirection(self, a, b, c, e):
        c.value = a
        e.value = b
        return a

    def test_enumColor2_allDirection(self, a, b, c, e):
        c.value = a
        e.value = b
        return a

    def test_enumErrorCode_allDirection(self, a, b, c, e):
        c.value = a
        e.value = b
        return a
        
###############################################################################
# Client
###############################################################################

## Individual test cases / eRPC calls
def runTesttest_enumColor_in_out(client):
    global gEnumColorAClient
    enumColorC = erpc.Reference()
    enumColorC.value = 0
    # send request to the server
    print('\r\neRPC request is sent to the server')
    client.test_enumColor_in(gEnumColorAClient)
    print('\r\neRPC request is sent to the server')
    client.test_enumColor_out(enumColorC)
    assert enumColorC.value == gEnumColorAClient

def runTestEnumsServiceID(client):
    assert 2 == test_enums.interface.IEnumsService.SERVICE_ID 

def runTesttest_enumColor_inout(client):
    global gEnumColorAClient
    global gEnumColorBClient
    enumColorE = erpc.Reference()
    enumColorE.value = gEnumColorAClient
    # send request to the server
    print('\r\neRPC request is sent to the server')
    client.test_enumColor_inout(enumColorE)
    assert enumColorE.value == gEnumColorBClient

def runTesttest_enumColor_return(client):
    global gEnumColorAClient
    global gEnumColorBClient
    enumColorR = 0
    # send request to the server
    print('\r\neRPC request is sent to the server')
    client.test_enumColor_in(gEnumColorBClient)
    print('\r\neRPC request is sent to the server')
    enumColorR = client.test_enumColor_return()
    assert gEnumColorAClient == enumColorR

def runTesttest_enumColor_allDirection(client):
    global gEnumColorAClient
    global gEnumColorBClient
    enumColorC = erpc.Reference()
    enumColorC.value = 0
    enumColorE = erpc.Reference()
    enumColorE.value = gEnumColorAClient
    enumColorR = 0
    # send request to the server
    print('\r\neRPC request is sent to the server')
    enumColorR = client.test_enumColor_allDirection(gEnumColorAClient, gEnumColorBClient, enumColorC, enumColorE)
    assert gEnumColorAClient == enumColorC.value
    assert gEnumColorBClient == enumColorE.value
    assert gEnumColorAClient == enumColorR

def runTesttest_enumColor2_allDirection(client):
    enumColor2A = test_enums.common.enumColor2.pink
    enumColor2B = test_enums.common.enumColor2.yellow
    enumColor2C = erpc.Reference()
    enumColor2C.value = 0
    enumColor2E = erpc.Reference()
    enumColor2E.value = enumColor2A
    enumColor2R = 0
    # send request to the server
    print('\r\neRPC request is sent to the server')
    enumColor2R = client.test_enumColor2_allDirection(enumColor2A, enumColor2B, enumColor2C, enumColor2E)
    assert enumColor2A == enumColor2C.value
    assert enumColor2B == enumColor2E.value
    assert enumColor2A == enumColor2R

def runTesttest_enumErrorCode_allDirection(client):
    enumErrorCodeA = test_enums.common.enumErrorCode.ERROR_NONE
    enumErrorCodeB = test_enums.common.enumErrorCode.ERROR_UNKNOWN
    enumErrorCodeC = erpc.Reference()
    enumErrorCodeC.value = 0
    enumErrorCodeE = erpc.Reference()
    enumErrorCodeE.value = enumErrorCodeA
    enumErrorCodeR = 0
    # send request to the server
    print('\r\neRPC request is sent to the server')
    enumErrorCodeR = client.test_enumErrorCode_allDirection(enumErrorCodeA, enumErrorCodeB, enumErrorCodeC, enumErrorCodeE)
    assert enumErrorCodeA == enumErrorCodeC.value
    assert enumErrorCodeB == enumErrorCodeE.value
    assert enumErrorCodeA == enumErrorCodeR

###############################################################################
# PYTESTS
###############################################################################
class TestEnums:
    server_cmd = [(os.path.abspath(__file__)).replace(os.sep, '/'), '-s']
    services = [test_enums.server.EnumsServiceService(EnumsTestServiceHandler()), test_unit_test_common.server.CommonService(CommonTestServiceHandler())]
    clientsConstructors = [test_enums.client.EnumsServiceClient, test_unit_test_common.client.CommonClient]
    runner = None

    def test_runTesttestPrepareEnv(self):
        TestEnums.runner = Runner('test_enums', TestEnums.server_cmd, TestEnums.services, TestEnums.clientsConstructors)

    def test_runTestEnumsServiceID(self):
        self.runner.runTest(test_enums.interface.IEnumsService.SERVICE_ID, runTestEnumsServiceID)

    def test_runTesttest_enumColor_in_out(self):
        self.runner.runTest(test_enums.interface.IEnumsService.SERVICE_ID, runTesttest_enumColor_in_out)

    def test_runTesttest_enumColor_inout(self):
        self.runner.runTest(test_enums.interface.IEnumsService.SERVICE_ID, runTesttest_enumColor_inout)

    def test_runTesttest_enumColor_return(self):
        self.runner.runTest(test_enums.interface.IEnumsService.SERVICE_ID, runTesttest_enumColor_return)

    def test_runTesttest_enumColor_allDirection(self):
        self.runner.runTest(test_enums.interface.IEnumsService.SERVICE_ID, runTesttest_enumColor_allDirection)

    def test_runTesttest_enumColor2_allDirection(self):
        self.runner.runTest(test_enums.interface.IEnumsService.SERVICE_ID, runTesttest_enumColor2_allDirection)
    
    def test_runTesttest_enumErrorCode_allDirection(self):
        self.runner.runTest(test_enums.interface.IEnumsService.SERVICE_ID, runTesttest_enumErrorCode_allDirection)
    
    def test_runTesttestQuit(self):
        self.runner.runTest(test_unit_test_common.interface.ICommon.SERVICE_ID, runTesttestQuit)

    def runTests(self):
        self.test_runTesttestPrepareEnv()
        self.test_runTestEnumsServiceID()
        self.test_runTesttest_enumColor_in_out()
        self.test_runTesttest_enumColor_inout()
        self.test_runTesttest_enumColor_return()
        self.test_runTesttest_enumColor_allDirection()
        self.test_runTesttest_enumColor2_allDirection()
        self.test_runTesttest_enumErrorCode_allDirection()
        self.test_runTesttestQuit()

###############################################################################
# Main
###############################################################################

if __name__ == "__main__":
    tester = TestEnums()
    tester.runTests()
