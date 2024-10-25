#!/usr/bin/python

# Copyright 2021-2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause

import erpc
import pytest

from .service.erpc_outputs.test import common
from .service.erpc_outputs.test.client import PointersServiceClient
from .service.erpc_outputs.test.interface import IPointersService
from .service.erpc_outputs.test.server import PointersServiceService


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

    return PointersServiceClient(client_manager)


@pytest.fixture(scope="module")
def services():
    return [PointersServiceService(ListsTestServiceHandler())]


###############################################################################
# Handlers
###############################################################################
class ListsTestServiceHandler(IPointersService):
    def sendReceivedInt32(self, received_list):
        send_list = []
        for y in range(len(received_list)):
            send_list.append(2 * received_list[y])
        return send_list

    def sendReceived2Int32(self, received_list):
        send_list = [[] for a in range(len(received_list))]
        for x in range(len(received_list)):
            for y in range(len(received_list[x])):
                send_list[x].append(2 * received_list[x][y])
        return send_list

    def sendReceivedEnum(self, received_list):
        send_list = []
        for y in range(len(received_list)):
            send_list.append(received_list[y])
        return send_list

    def sendReceived2Enum(self, received_list):
        send_list = [[] for a in range(len(received_list))]
        for x in range(len(received_list)):
            for y in range(len(received_list[x])):
                send_list[x].append(received_list[x][y])
        return send_list

    def sendReceivedStruct(self, received_list):
        send_list = [common.C() for b in range(len(received_list))]
        for y in range(len(received_list)):
            send_list[y].m = 2 * received_list[y].m
            send_list[y].n = 2 * received_list[y].n
        return send_list

    def sendReceived2Struct(self, received_list):
        send_list = [[] for a in range(len(received_list))]
        for x in range(len(received_list)):
            send_list[x] = [common.C() for b in range(len(received_list[x]))]
            for y in range(len(received_list[x])):
                send_list[x][y].m = 2 * received_list[x][y].m
                send_list[x][y].n = 2 * received_list[x][y].n
        return send_list

    def sendReceivedString(self, received_list):
        send_list = []
        for y in range(len(received_list)):
            send_list.append(received_list[y] + " World!")
        return send_list

    def sendReceived2String(self, received_list):
        send_list = [[] for a in range(len(received_list))]
        for x in range(len(received_list)):
            for y in range(len(received_list[x])):
                send_list[x].append(received_list[x][y] + " World!")
        return send_list

    @pytest.mark.skip(reason="They do not belong to the tests.")
    def test_list_allDirection(self, a, b, e):
        # e list is defined as an inout parameter
        for y in range(len(a)):
            e.value[y] = 2 * e.value[y]
        return

    def testLengthAnnotation(self, myList):
        for y in range(len(myList)):
            if myList[y] != y + 1:
                return 0
        return 1

    def testLengthAnnotationInStruct(self, s):
        for y in range(s.len):
            if s.myList[y] != y + 1:
                return 0
        return 10

    def returnSentStructLengthAnnotation(self, s):
        ret = common.listStruct([])
        for x in range(s.len):
            ret.myList.append(s.myList[x])
        return ret

    def sendGapAdvertisingData(self, ad):
        for x in range(ad.cNumAdStructures):
            if ad.aAdStructures[x].adType != 5:
                return 0
            for y in range(ad.aAdStructures[x].length):
                if ad.aAdStructures[x].aData[y] != ((y + 1) * 5):
                    return 0
        return 33


###############################################################################
# Tests
###############################################################################
def test_ListsServiceID(client):
    assert 2 == IPointersService.SERVICE_ID


def test_SendReceivedInt32(client):
    send_list = range(12)
    print('\r\neRPC request is sent to the server')
    received_list = client.sendReceivedInt32(send_list)
    for x in range(len(received_list)):
        assert (received_list[x] / 2) == send_list[x]


def test_SendReceiveZeroSize(client):
    send_list = [0]
    print('\r\neRPC request is sent to the server')
    received_list = client.sendReceivedInt32(send_list)
    assert received_list == [0]


def test_SendReceived2Int32(client):
    send_list = [[0] for a in range(10)]
    for a in range(10):
        send_list[a] = range(a + 1)
    # print(send_list)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    received_list = client.sendReceived2Int32(send_list)
    # print(received_list)
    for x in range(len(received_list)):
        for y in range(len(received_list[x])):
            assert (received_list[x][y] / 2) == send_list[x][y]


def test_SendReceivedEnum(client):
    send_list = []
    send_list.append(common.enumColor.red)
    send_list.append(common.enumColor.green)
    send_list.append(common.enumColor.blue)
    print('\r\neRPC request is sent to the server')
    received_list = client.sendReceivedEnum(send_list)
    # print(received_list)
    for x in range(len(received_list)):
        assert received_list[x] == send_list[x]


def test_SendReceived2Enum(client):
    send_list = [[] for a in range(3)]
    for a in range(3):
        for b in range(a + 1):
            if b == 0:
                send_list[a].append(common.enumColor.red)
            if b == 1:
                send_list[a].append(common.enumColor.green)
            if b == 2:
                send_list[a].append(common.enumColor.blue)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    received_list = client.sendReceived2Enum(send_list)
    # print(received_list)
    for x in range(len(received_list)):
        assert received_list[x] == send_list[x]


def test_SendReceivedStruct(client):
    send_list = []
    for a in range(10):
        send_list.append(common.C(a, a * 2))
    # print(send_list)
    print('\r\neRPC request is sent to the server')
    received_list = client.sendReceivedStruct(send_list)
    # print(received_list)
    for x in range(len(received_list)):
        assert (received_list[x].m) / 2 == send_list[x].m
        assert (received_list[x].n) / 2 == send_list[x].n


def test_SendReceived2Struct(client):
    send_list = [[] for a in range(10)]
    for a in range(10):
        for b in range(a + 1):
            send_list[a].append(common.C(a, b))
    # print(send_list)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    received_list = client.sendReceived2Struct(send_list)
    # print(received_list)
    for x in range(len(received_list)):
        for y in range(len(received_list[x])):
            assert (received_list[x][y].m) / 2 == send_list[x][y].m
            assert (received_list[x][y].n) / 2 == send_list[x][y].n


def test_SendReceivedString(client):
    send_list = ["Hello" for a in range(12)]
    # print(send_list)
    print('\r\neRPC request is sent to the server')
    received_list = client.sendReceivedString(send_list)
    # print(received_list)
    for x in range(len(received_list)):
        assert received_list[x] == "Hello World!"


def test_SendReceived2String(client):
    send_list = [[] for a in range(5)]
    for a in range(5):
        for b in range(a + 1):
            send_list[a].append("Hello")
    # print(send_list)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    received_list = client.sendReceived2String(send_list)
    # print(received_list)
    for x in range(len(received_list)):
        for y in range(len(received_list[x])):
            assert received_list[x][y] == "Hello World!"


def test_test_list_allDirection(client):
    send_list_a = []
    send_list_b = []
    send_list_e = erpc.Reference()
    send_list_e.value = []
    for x in range(10):
        send_list_a.append(x)
        send_list_b.append(2 * x)
        send_list_e.value.append(6 * x)
    # print(send_list_e)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    client.test_list_allDirection(send_list_a, send_list_b, send_list_e)
    # print(send_list_e)
    for x in range(len(send_list_a)):
        assert send_list_e.value[x] == 12 * send_list_a[x]


def test_testLengthAnnotation(client):
    length = 5
    list = []
    for x in range(length):
        list.append(x + 1)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    result = client.testLengthAnnotation(list)
    assert result == 1


def test_testLengthAnnotationInStruct(client):
    myListStruct = common.listStruct([])
    for x in range(5):
        myListStruct.myList.append(x + 1)
    # print(myListStruct)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    result = client.testLengthAnnotationInStruct(myListStruct)
    assert result == 10


def test_returnSentStructLengthAnnotation(client):
    myListStruct = common.listStruct([])
    returnStruct = common.listStruct([])
    for x in range(5):
        myListStruct.myList.append(x + 1)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    returnStruct = client.returnSentStructLengthAnnotation(myListStruct)
    for x in range(returnStruct.len):
        assert myListStruct.myList[x] == returnStruct.myList[x]


def test_sendGapAdvertisingData(client):
    ad = common.gapAdvertisingData_t([])
    for x in range(2):
        ad.aAdStructures.append(common.gapAdStructure_t(adType=5, aData=[((a + 1) * 5) for a in range(3)]))
    # print(ad)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    result = client.sendGapAdvertisingData(ad)
    assert result == 33


def test_Quit(common_client):
    # send request to the server
    print("eRPC quit request send to the server.")
    return common_client.quit()
