#!/usr/bin/python

# Copyright 2021-2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause

import erpc
import pytest

from .service.erpc_outputs.test.client import BinaryClient
from .service.erpc_outputs.test.interface import IBinary
from .service.erpc_outputs.test.server import BinaryService


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

    return BinaryClient(client_manager)


@pytest.fixture(scope="module")
def services():
    return [BinaryService(BinaryTestServiceHandler())]


###############################################################################
# Handlers
###############################################################################
class BinaryTestServiceHandler(IBinary):
    def sendBinary(self, a):
        # print(a)
        return

    def test_binary_allDirection(self, a, b, e):
        e.value = bytearray(b'')
        for x in range(5):
            e.value.append(a[x] * b[x])
        return

    def test_binary_allDirectionLength(self, a, b, d):
        return


###############################################################################
# Tests
###############################################################################
def test_sendBinary(client):
    send_binary = b'01234'
    # send request to the server
    print('\r\neRPC request is sent to the server')
    client.sendBinary(send_binary)


def test_BinaryServiceID(client):
    assert 2 == IBinary.SERVICE_ID


def test_binary_allDirection(client):
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
    # print(e_binary)
    for x in range(5):
        assert e_binary.value[x] == (a_binary[x] * b_binary[x])


def test_Quit(common_client):
    # send request to the server
    print("eRPC quit request send to the server.")
    return common_client.quit()
