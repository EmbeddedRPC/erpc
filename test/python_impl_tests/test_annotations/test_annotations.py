#!/usr/bin/python

# Copyright 2021-2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause

import sys

import pytest

from .service.test.client import AnnotateTestClient
from .service.test.server import AnnotateTestService
from .service.test.interface import IAnnotateTest


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

    return AnnotateTestClient(client_manager)


@pytest.fixture(scope="module")
def services():
    return [AnnotateTestService(AnnotateTestServiceHandler())]


###############################################################################
# Handlers
###############################################################################
class AnnotateTestServiceHandler(IAnnotateTest):

    def add(self, a, b):
        print('\r\add service reached: a=', a, 'b=', b, '\r\n=============')
        sys.stdout.flush()
        return a + b

    def testIfFooStructExist(self, a):
        return

    def testIfMyEnumExist(self, a):
        return

    def testIfMyIntAndConstExist(self, a):
        return a


###############################################################################
# Tests
###############################################################################
@pytest.mark.skip("Not implemented.")
def test_IncludeAnnotationCheck(client):
    # send request to the server
    print('\r\neRPC request is sent to the server')
    assert 5 == client.addOne(4)


def test_AnnotationServiceID(client):
    assert 5 == IAnnotateTest.SERVICE_ID


def test_IfMyIntAndConstExist(client, server):
    # send request to the server
    print('\r\neRPC request is sent to the server')
    assert 1 == client.testIfMyIntAndConstExist(1)


def test_Quit(common_client):
    # send request to the server
    print("eRPC quit request send to the server.")
    return common_client.quit()
