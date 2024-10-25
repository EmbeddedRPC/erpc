#!/usr/bin/python

# Copyright 2021-2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause

import pytest

from .service.erpc_outputs.test import common
from .service.erpc_outputs.test.client import TypedefServiceClient
from .service.erpc_outputs.test.interface import ITypedefService
from .service.erpc_outputs.test.server import TypedefServiceService


###############################################################################
# Variables
###############################################################################


###############################################################################
# Fixtures
###############################################################################
@pytest.fixture()
def client(client_manager):
    if client_manager is None:
        pytest.skip()

    return TypedefServiceClient(client_manager)


@pytest.fixture(scope="module")
def services():
    return [TypedefServiceService(TypedefTestServiceHandler())]


###############################################################################
# Handlers
###############################################################################
class TypedefTestServiceHandler(ITypedefService):
    def sendReceiveInt(self, a):
        return a * 2 + 1

    def sendReceiveEnum(self, a):
        return a + 1

    def sendReceiveStruct(self, a):
        b = common.B(m=(2 * a.m), n=(2 + a.n))
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
# Tests
###############################################################################
def test_SendReceiveInt(client):
    a = 10
    b = 2 * a + 1
    # send request to the server
    print('\r\neRPC request is sent to the server')
    assert b == client.sendReceiveInt(a)


def test_TestTypedefServiceID(client):
    assert 2 == ITypedefService.SERVICE_ID


def test_SendReceiveEnum(client):
    a = common.Colors.green
    # send request to the server
    print('\r\neRPC request is sent to the server')
    assert common.Colors.blue == client.sendReceiveEnum(a)


def test_SendReceiveStruct(client):
    a = common.B(m=1, n=4)
    b = common.B(m=(2 * a.m), n=(2 + a.n))
    r = common.B()
    # send request to the server
    print('\r\neRPC request is sent to the server')
    r = client.sendReceiveStruct(a)
    assert r.m == b.m
    assert r.n == b.n


def test_SendReceiveListType(client):
    send_list = range(12)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    received_list = client.sendReceiveListType(send_list)
    for x in range(len(received_list)):
        assert (received_list[x] / 2) == send_list[x]


def test_SendReceiveString(client):
    send = "Hello"
    print('\r\neRPC request is sent to the server')
    received = client.sendReceiveString(send)
    assert received == "Hello World!"


def test_SendReceive2ListType(client):
    send_list = [[0] for a in range(10)]
    for a in range(10):
        send_list[a] = range(a + 1)
    # print(send_list)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    received_list = client.sendReceive2ListType(send_list)
    # print(received_list)
    for x in range(len(received_list)):
        for y in range(len(received_list[x])):
            assert (received_list[x][y] / 2) == send_list[x][y]


def test_Quit(common_client):
    # send request to the server
    print("eRPC quit request send to the server.")
    return common_client.quit()
