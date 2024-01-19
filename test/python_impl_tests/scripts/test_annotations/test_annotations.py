#!/usr/bin/python

# Copyright 2021 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import inspect, os, sys
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))))
from server.runner import Runner
from service.test_annotations import test as test_annotations
from service.test_annotations import test_unit_test_common
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
class AnnotateTestServiceHandler(test_annotations.interface.IAnnotateTest):
    ## eRPC add method
    # @param a First addend
    # @param b Second addend
    def add(self, a, b):
        print('\r\add service reached: a=',a,'b=',b, '\r\n=============')
        sys.stdout.flush()
        return a + b
        
    def testIfFooStructExist(self, a):
        return

    def testIfMyEnumExist(self, a):
        return

    def testIfMyIntAndConstExist(self, a):
        return a

###############################################################################
# Client
###############################################################################

def runTestIncludeAnnotationCheck(client):
    # send request to the server
    print('\r\neRPC request is sent to the server')
    assert 5 == client.addOne(4)

## Individual test cases / eRPC calls
def runTestAnnotationServiceID(client):
    assert 5 == test_annotations.interface.IAnnotateTest.SERVICE_ID 

def runTesttestIfMyIntAndConstExist(client):
    # send request to the server
    print('\r\neRPC request is sent to the server')
    assert 1 == client.testIfMyIntAndConstExist(1)

###############################################################################
# PYTESTS
###############################################################################
class TestAnnotation:
    server_cmd = [(os.path.abspath(__file__)).replace(os.sep, '/'), '-s']
    services = [test_annotations.server.AnnotateTestService(AnnotateTestServiceHandler()), test_unit_test_common.server.CommonService(CommonTestServiceHandler())]
    clientsConstructors = [test_annotations.client.AnnotateTestClient, test_unit_test_common.client.CommonClient]
    runner = None

    def test_runTesttestPrepareEnv(self):
        TestAnnotation.runner = Runner('test_annotations', TestAnnotation.server_cmd, TestAnnotation.services, TestAnnotation.clientsConstructors)

    def test_runTestAnnotationServiceID(self):
        self.runner.runTest(test_annotations.interface.IAnnotateTest.SERVICE_ID , runTestAnnotationServiceID)

    def test_runTesttestIfMyIntAndConstExist(self):
        self.runner.runTest(test_annotations.interface.IAnnotateTest.SERVICE_ID , runTesttestIfMyIntAndConstExist)

    def test_runTesttestQuit(self):
        self.runner.runTest(test_unit_test_common.interface.ICommon.SERVICE_ID, runTesttestQuit)

    def runTests(self):
        self.test_runTesttestPrepareEnv()
        self.test_runTestAnnotationServiceID()
        self.test_runTesttestIfMyIntAndConstExist()
        self.test_runTesttestQuit()

#TBD runTestIncludeAnnotationCheck(client)

###############################################################################
# Main
###############################################################################

if __name__ == "__main__":
    #sys.exit(pytest.main(["-qq"], plugins=[TestAnnotation.close()]))
    tester = TestAnnotation()
    tester.runTests()
    
