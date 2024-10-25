#!/usr/bin/python

# Copyright 2021-2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause

import erpc
import pytest

from .service.erpc_outputs.test import common
from .service.erpc_outputs.test.client import EnumsServiceClient
from .service.erpc_outputs.test.interface import IEnumsService
from .service.erpc_outputs.test.server import EnumsServiceService


###############################################################################
# Variables
###############################################################################
gEnumColorAClient = common.enumColor.green
gEnumColorBClient = common.enumColor.red
gEnumColorAServer = 0
gEnumColorBServer = 0


###############################################################################
# Fixtures
###############################################################################
@pytest.fixture()
def client(client_manager):
    if client_manager is None:
        pytest.skip()

    return EnumsServiceClient(client_manager)


@pytest.fixture(scope="module")
def services():
    return [EnumsServiceService(EnumsTestServiceHandler())]


###############################################################################
# Handlers
###############################################################################
class EnumsTestServiceHandler(IEnumsService):
    def test_enumColor_in(self, a):
        global gEnumColorAServer
        gEnumColorAServer = a
        return

    def test_enumColor_in2(self, b):
        global gEnumColorBServer
        gEnumColorBServer = b
        return

    def test_enumColor_out(self, c):
        # c enum is defined as an out parameter
        c.value = gEnumColorAServer
        return

    def test_enumColor_inout(self, e):
        # e enum is defined as an out parameter
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
# Tests
###############################################################################

def test_enumColor_in_out(client):
    global gEnumColorAClient
    enumColorC = erpc.Reference()
    enumColorC.value = 0
    # send request to the server
    print('\r\neRPC request is sent to the server')
    client.test_enumColor_in(gEnumColorAClient)
    print('\r\neRPC request is sent to the server')
    client.test_enumColor_out(enumColorC)
    assert enumColorC.value == gEnumColorAClient


def test_EnumsServiceID(client):
    assert 2 == IEnumsService.SERVICE_ID


def test_enumColor_inout(client):
    global gEnumColorAClient
    global gEnumColorBClient
    enumColorE = erpc.Reference()
    enumColorE.value = gEnumColorAClient
    # send request to the server
    print('\r\neRPC request is sent to the server')
    client.test_enumColor_inout(enumColorE)
    assert enumColorE.value == gEnumColorBClient


def test_enumColor_return(client):
    global gEnumColorAClient
    global gEnumColorBClient
    enumColorR = 0
    # send request to the server
    print('\r\neRPC request is sent to the server')
    client.test_enumColor_in(gEnumColorBClient)
    print('\r\neRPC request is sent to the server')
    enumColorR = client.test_enumColor_return()
    assert gEnumColorAClient == enumColorR


def test_enumColor_allDirection(client):
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


def test_enumColor2_allDirection(client):
    enumColor2A = common.enumColor2.pink
    enumColor2B = common.enumColor2.yellow
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


def test_enumErrorCode_allDirection(client):
    enumErrorCodeA = common.enumErrorCode.ERROR_NONE
    enumErrorCodeB = common.enumErrorCode.ERROR_UNKNOWN
    enumErrorCodeC = erpc.Reference()
    enumErrorCodeC.value = 0
    enumErrorCodeE = erpc.Reference()
    enumErrorCodeE.value = enumErrorCodeA
    enumErrorCodeR = 0
    # send request to the server
    print('\r\neRPC request is sent to the server')
    enumErrorCodeR = client.test_enumErrorCode_allDirection(enumErrorCodeA, enumErrorCodeB, enumErrorCodeC,
                                                            enumErrorCodeE)
    assert enumErrorCodeA == enumErrorCodeC.value
    assert enumErrorCodeB == enumErrorCodeE.value
    assert enumErrorCodeA == enumErrorCodeR


def test_Quit(common_client):
    # send request to the server
    print("eRPC quit request send to the server.")
    return common_client.quit()
