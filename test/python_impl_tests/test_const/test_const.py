#!/usr/bin/python
import time

# Copyright 2021-2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause


import pytest

from .service.erpc_outputs.test import common
from .service.erpc_outputs.test.client import EmptyInterfaceClient
from .service.erpc_outputs.test.interface import IEmptyInterface
from .service.erpc_outputs.test.server import EmptyInterfaceService


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

    return EmptyInterfaceClient(client_manager)


@pytest.fixture(scope="module")
def services():
    return [EmptyInterfaceService(ConstTestServiceHandler())]


###############################################################################
# Handlers
###############################################################################
class ConstTestServiceHandler(IEmptyInterface):
    pass


###############################################################################
# Tests
###############################################################################
def test_CheckConsts(client):
    assert common.a == 3
    assert common.b == 3.14
    assert common.c == "feedbabe"
    assert common.d == 3.14
    assert common.x == 11
    assert common.y == 20
    assert common.mass == 100
    assert common.accel == -9.8


def test_ConstServiceID(client):
    assert 2 == IEmptyInterface.SERVICE_ID


def test_Quit(common_client):
    # send request to the server
    print("eRPC quit request send to the server.")
    return common_client.quit()
