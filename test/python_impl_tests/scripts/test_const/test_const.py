#!/usr/bin/python

# Copyright 2021 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import inspect, os, sys
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))))
from server.runner import Runner
from service.test_const.erpc_outputs import test as test_const
from service.test_const.erpc_outputs import test_unit_test_common
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
class ConstTestServiceHandler(test_const.interface.IEmptyInterface):
    pass

###############################################################################
# Client
###############################################################################

## Individual test cases / eRPC calls
def runTestCheckConsts(client):
    assert test_const.common.a == 3
    assert test_const.common.b == 3.14
    assert test_const.common.c == "feedbabe"
    assert test_const.common.d == 3.14
    assert test_const.common.x == 11
    assert test_const.common.y == 20
    assert test_const.common.mass == 100
    assert test_const.common.accel == -9.8

def runTestConstServiceID(client):
    assert 2 == test_const.interface.IEmptyInterface.SERVICE_ID 

###############################################################################
# PYTESTS
###############################################################################
class TestConst:
    server_cmd = [(os.path.abspath(__file__)).replace(os.sep, '/'), '-s']
    services = [test_const.server.EmptyInterfaceService(ConstTestServiceHandler()), test_unit_test_common.server.CommonService(CommonTestServiceHandler())]
    clientsConstructors = [test_const.client.EmptyInterfaceClient, test_unit_test_common.client.CommonClient]
    runner = None

    def test_runTesttestPrepareEnv(self):
        TestConst.runner = Runner('test_const', TestConst.server_cmd, TestConst.services, TestConst.clientsConstructors)

    def test_runTestConstServiceID(self):
        self.runner.runTest(test_const.interface.IEmptyInterface.SERVICE_ID, runTestConstServiceID)

    def test_runTestCheckConsts(self):
        self.runner.runTest(test_const.interface.IEmptyInterface.SERVICE_ID, runTestCheckConsts)

    def test_runTesttestQuit(self):
        self.runner.runTest(test_unit_test_common.interface.ICommon.SERVICE_ID, runTesttestQuit)

    def runTests(self):
        self.test_runTesttestPrepareEnv()
        self.test_runTesttestQuit()
        self.test_runTestConstServiceID()
        self.test_runTestCheckConsts()

###############################################################################
# Main
###############################################################################

if __name__ == "__main__":
    tester = TestConst()
    tester.runTests()
