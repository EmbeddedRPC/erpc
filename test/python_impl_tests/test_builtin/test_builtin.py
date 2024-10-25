#!/usr/bin/python

# Copyright 2021-2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause

import erpc
import pytest

from .service.erpc_outputs.test.client import BuiltinServicesClient
from .service.erpc_outputs.test.interface import IBuiltinServices
from .service.erpc_outputs.test.server import BuiltinServicesService

###############################################################################
# Variables
###############################################################################
gInt32AClient = 2
gInt32BClient = -20
gStringAClient = "Hello"
gStringBClient = "World!"
gSaveVariablesServer = {'int32A': 0, 'int32B': 0, 'stringA': "", 'stringB': ""}


###############################################################################
# Fixtures
###############################################################################
@pytest.fixture()
def client(client_manager):
    if client_manager is None:
        pytest.skip()

    return BuiltinServicesClient(client_manager)


@pytest.fixture(scope="module")
def services():
    return [BuiltinServicesService(BuiltinTestServiceHandler())]


###############################################################################
# Handlers
###############################################################################
class BuiltinTestServiceHandler(IBuiltinServices):
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
        # c integer is defined as an out parameter
        c.value = gSaveVariablesServer['int32A']
        return

    def test_int32_inout(self, e):
        # e integer is defined as an out parameter
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
        # b float is defined as an out parameter and needs to be initialized first
        b.value = 0.0
        b.value = a
        return

    def test_double_inout(self, a, b):
        # b double is defined as an out parameter and needs to be initialized first
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
        # c string is defined as an out parameter
        c.value = gSaveVariablesServer['stringA']
        return

    def test_string_inout(self, e):
        global gSaveVariablesServer
        # e string is defined as an inout parameter
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
# Tests
###############################################################################
def test_int32_in_out(client):
    global gInt32AClient
    int32C = erpc.Reference()
    int32C.value = 0
    # send request to the server
    print('\r\neRPC request is sent to the server')
    client.test_int32_in(gInt32AClient)
    print('\r\neRPC request is sent to the server')
    client.test_int32_out(int32C)
    assert gInt32AClient == int32C.value


def test_BuiltinServiceID(client):
    assert 2 == IBuiltinServices.SERVICE_ID


def test_int32_inout(client):
    int32E = erpc.Reference()
    int32E.value = -6
    for a in range(-5, 5):
        # send request to the server
        print('\r\neRPC request is sent to the server')
        client.test_int32_inout(int32E)
        assert a == int32E.value


def test_int32_return(client):
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


def test_int32_allDirection(client):
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


def test_float_inout(client):
    a = float(3.14)
    b = erpc.Reference(0.00)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    client.test_float_inout(a, b)
    assert round(b.value, 2) == a


def test_double_inout(client):
    a = float(3.14)
    b = erpc.Reference(0.00)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    client.test_double_inout(a, b)
    assert b.value == a


def test_string_in_out(client):
    global gStringAClient
    stringC = erpc.Reference()
    stringC.value = ""
    # send request to the server
    print('\r\neRPC request is sent to the server')
    client.test_string_in(gStringAClient)
    print('\r\neRPC request is sent to the server')
    client.test_string_out(stringC)
    assert gStringAClient == stringC.value


def test_string_inout(client):
    stringE = erpc.Reference()
    stringE.value = gStringAClient + " "
    # send request to the server
    print('\r\neRPC request is sent to the server')
    client.test_string_in2(gStringBClient)
    print('\r\neRPC request is sent to the server')
    client.test_string_inout(stringE)
    assert "Hello World!" == stringE.value


def test_string_return(client):
    stringR = ""
    # send request to the server
    print('\r\neRPC request is sent to the server')
    stringR = client.test_string_return()
    assert "Hello World!" == stringR


def test_string_allDirection(client):
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


def test_StringParamTest1(client):
    int32R = -1
    # send request to the server
    print('\r\neRPC request is sent to the server')
    int32R = client.sendHello("Hello World!")
    assert int32R == 0


def test_StringParamTest2(client):
    int32R = -1
    # send request to the server
    print('\r\neRPC request is sent to the server')
    int32R = client.sendTwoStrings("String one.", "String two.")
    assert int32R == 0


def test_StringReturnTest1(client):
    # send request to the server
    print('\r\neRPC request is sent to the server')
    assert "Hello" == client.returnHello()


def test_Quit(common_client):
    # send request to the server
    print("eRPC quit request send to the server.")
    return common_client.quit()
