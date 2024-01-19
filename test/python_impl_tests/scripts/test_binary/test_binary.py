#!/usr/bin/python

# Copyright 2021 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import inspect, os, sys
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))))
from server.runner import Runner
from service.test_binary.erpc_outputs import test as test_binary
from service.test_binary.erpc_outputs import test_unit_test_common
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
class BinaryTestServiceHandler(test_binary.interface.IBinary):
    def sendBinary(self, a):
        #print(a)
        return

    def test_binary_allDirection(self, a, b, e):
        e.value = bytearray(b'')
        for x in range(5):
            e.value.append(a[x] * b[x])
        return

    def test_binary_allDirectionLength(self, a, b, d, p1):
        return

###############################################################################
# Client
###############################################################################

## Individual test cases / eRPC calls
def runTestsendBinary(client):
    send_binary = b'01234'
    # send request to the server
    print('\r\neRPC request is sent to the server')
    client.sendBinary(send_binary)

def runTestBinaryServiceID(client):
    assert 2 == test_binary.interface.IBinary.SERVICE_ID 

def runTesttest_binary_allDirection(client):
    a_binary = bytearray(b'')
    b_binary = bytearray(b'')
    e_binary = erpc.Reference()
    e_binary.value = bytearray(b'')
    for x in range(5):
        a_binary.append(x)
        b_binary.append(x * 2)
        e_binary.value.append(x)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    client.test_binary_allDirection(a_binary, b_binary, e_binary)
    #print(e_binary)
    for x in range(5):
        assert e_binary.value[x] == (a_binary[x] * b_binary[x]) 

###############################################################################
# PYTESTS
###############################################################################
class TestBinary:
    server_cmd = [(os.path.abspath(__file__)).replace(os.sep, '/'), '-s']
    services = [test_binary.server.BinaryService(BinaryTestServiceHandler()), test_unit_test_common.server.CommonService(CommonTestServiceHandler())]
    clientsConstructors = [test_binary.client.BinaryClient, test_unit_test_common.client.CommonClient]
    runner = None

    def test_runTesttestPrepareEnv(self):
        TestBinary.runner = Runner('test_binary', TestBinary.server_cmd, TestBinary.services, TestBinary.clientsConstructors)

    def test_runTestBinaryServiceID(self):
        self.runner.runTest(test_binary.interface.IBinary.SERVICE_ID, runTestBinaryServiceID)

    def test_runTestsendBinary(self):
        self.runner.runTest(test_binary.interface.IBinary.SERVICE_ID, runTestsendBinary)

    def test_runTesttest_binary_allDirection(self):
        self.runner.runTest(test_binary.interface.IBinary.SERVICE_ID, runTesttest_binary_allDirection)

    def test_runTesttestQuit(self):
        self.runner.runTest(test_unit_test_common.interface.ICommon.SERVICE_ID, runTesttestQuit)
    
    def runTests(self):
        self.test_runTesttestPrepareEnv()
        self.test_runTestBinaryServiceID()
        self.test_runTestsendBinary()
        self.test_runTesttest_binary_allDirection()
        self.test_runTesttestQuit()

###############################################################################
# Main
###############################################################################

if __name__ == "__main__":
    tester = TestBinary()
    tester.runTests()
