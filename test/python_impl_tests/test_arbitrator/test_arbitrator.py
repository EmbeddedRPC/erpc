#!/usr/bin/python

# Copyright 2021-2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause

from __future__ import annotations

import multiprocessing
import threading
import time
from multiprocessing import Process
from typing import Optional

import pytest
from _pytest.config import Config
from erpc.arbitrator import TransportArbitrator
from erpc.basic_codec import BasicCodec
from erpc.client import ClientManager
from erpc.server import Service
from erpc.simple_server import SimpleServer, ServerThread

from .service.erpc_outputs.test_firstInterface.interface import IFirstInterface
from .service.erpc_outputs.test_secondInterface.interface import ISecondInterface
from .service.erpc_outputs.test_firstInterface.client import FirstInterfaceClient
from .service.erpc_outputs.test_secondInterface.client import SecondInterfaceClient
from .service.erpc_outputs.test_firstInterface.server import FirstInterfaceService
from .service.erpc_outputs.test_secondInterface.server import SecondInterfaceService
from ..conftest import TransportFactory

###############################################################################
# Variables
###############################################################################
NUMBER = 15
NESTED_CALLS_COUNT = 10

client_j: int = 0
numbers_client: list[int] = [0 for _ in range(NUMBER)]  # numbers[NUMBER];
enabled: bool = False

gc_server: Optional[SimpleServer] = None
gc_client: Optional[FirstInterfaceClient] = None
gc_transport: Optional[TransportArbitrator] = None
gc_lock = threading.Lock()
gc_wait_quit = 0
gc_nested_calls_count = 0

server_i: int = 0
numbers_server: list[int] = [0 for _ in range(NUMBER)]  # numbers[NUMBER];

gs_server: Optional[SimpleServer] = None
gs_client: Optional[SecondInterfaceClient] = None
gs_transport: Optional[TransportArbitrator] = None
gs_lock = threading.Lock()
gs_wait_quit = 0
gs_wait_client = 0
gs_is_test_passing = 0
gs_stop_test = 0


###############################################################################
# Fixtures
###############################################################################

@pytest.fixture(scope="module")
def client_manager() -> None:
    pass


@pytest.fixture(scope="module")
def services() -> list[Service]:
    return []


@pytest.fixture(scope="module")
def client(config: Config, transport_factory: TransportFactory, server) -> FirstInterfaceClient:
    global gc_server, gc_client, gc_transport, gc_wait_quit

    if config.getoption("client") is False:
        pytest.skip("No client")

    time.sleep(0.1)

    gc_transport = transport_factory.build_transport(False, True)

    client_mngr = ClientManager(gc_transport.shared_transport, BasicCodec)
    client_mngr.arbitrator = gc_transport

    service = SecondInterfaceService(
        SecondInterfaceHandler()
    )

    gc_server = SimpleServer(gc_transport, BasicCodec)
    gc_server.add_service(service)

    server_thread = threading.Thread(target=client_server, args=())
    server_thread.start()

    gc_client = FirstInterfaceClient(client_mngr)

    gc_client.whenReady()

    yield gc_client  # run all tests

    while True:
        with gc_lock:
            if gc_wait_quit != 0:
                break
        time.sleep(0.1)

    # send ready to quit state to server (second side)
    gc_client.quitFirstInterfaceServer()

    client_increase_wait_quit()

    # wait until server and client will stop.
    while True:
        with gc_lock:
            if gc_wait_quit >= 3:
                break
        time.sleep(0.1)

    # close transport
    gc_transport.shared_transport.close()


@pytest.fixture(scope="module", autouse=True)
def server(config: Config, transport_factory: TransportFactory) -> int:
    global gs_server, gs_transport, gs_client

    if not config.getoption("server"):
        yield 0
        return

    if config.getoption("client"):
        # Run in new process
        p = Process(target=server_process, args=(transport_factory,))
        p.start()
        yield 0
        p.join()
    else:
        yield server_process(transport_factory)


###############################################################################
# Server process
###############################################################################

def server_process(transport_factory: TransportFactory) -> int:
    global gs_server, gs_transport, gs_client
    gs_transport = transport_factory.build_transport(True, True)

    client_mngr = ClientManager(gs_transport.shared_transport, BasicCodec)
    client_mngr.arbitrator = gs_transport

    gs_client = SecondInterfaceClient(client_mngr)

    service = FirstInterfaceService(
        FirstInterfaceHandler()
    )

    gs_server = SimpleServer(gs_transport, BasicCodec)
    gs_server.add_service(service)

    server_thread = threading.Thread(target=server_server, args=())
    client_thread = threading.Thread(target=server_client, args=())

    server_thread.start()
    client_thread.start()

    while True:
        with gs_lock:
            if gs_wait_quit >= 3:
                break
        time.sleep(0.1)

    gs_transport.shared_transport.close()

    return gs_is_test_passing


###############################################################################
# Functions
###############################################################################

def client_server() -> None:
    global gc_server

    gc_server.run()
    client_increase_wait_quit()


def client_increase_wait_quit():
    global gc_wait_quit

    with gc_lock:
        gc_wait_quit += 1


def server_client() -> None:
    global gs_is_test_passing

    while gs_wait_client == 0:
        pass

    while True:
        gs_is_test_passing = server_test_client()
        with gs_lock:
            if gs_wait_quit != 0 or gs_is_test_passing != 0 or gs_stop_test != 0:
                gs_client.enableFirstSide()
                break
        time.sleep(0.1)

    while True:
        with gs_lock:
            if gs_wait_quit != 0:
                break
        time.sleep(0.1)

    gs_client.quitSecondInterfaceServer()

    server_increase_wait_quit()


def server_server() -> None:
    gs_server.run()
    server_increase_wait_quit()


def server_increase_wait_quit():
    global gs_wait_quit

    with gs_lock:
        gs_wait_quit += 1


def server_test_client():
    number = 15

    for i in range(number):
        gs_client.secondSendInt(i + number)

    for i in range(number - 1, -1, -1):
        if i + number != gs_client.secondReceiveInt():
            return -1

    return 0


###############################################################################
# Handlers
###############################################################################

class SecondInterfaceHandler(ISecondInterface):
    nested_calls_count = 0

    def secondSendInt(self, a):
        global client_j
        numbers_client[client_j] = a
        client_j += 1

    def secondReceiveInt(self):
        global client_j

        client_j -= 1
        return numbers_client[client_j]

    def quitSecondInterfaceServer(self):
        global gc_server, gc_wait_quit
        gc_server.stop()

        client_increase_wait_quit()

    def enableFirstSide(self):
        global enabled
        enabled = True

    def callFirstSide(self):
        global gc_nested_calls_count
        gc_nested_calls_count += 1

        if gc_nested_calls_count < NESTED_CALLS_COUNT:
            return gc_client.callSecondSide() + 1

        return 0


class FirstInterfaceHandler(IFirstInterface):

    def whenReady(self):
        global gs_wait_client
        gs_wait_client += 1

    def firstSendInt(self, a):
        global server_i
        numbers_server[server_i] = a
        server_i += 1

    def firstReceiveInt(self):
        global server_i
        server_i -= 1
        return numbers_server[server_i]

    def stopSecondSide(self):
        global gs_stop_test
        gs_stop_test += 1

    def getResultFromSecondSide(self):
        global gs_is_test_passing
        return gs_is_test_passing

    def testCasesAreDone(self):
        server_increase_wait_quit()

    def quitFirstInterfaceServer(self):
        gs_server.stop()

    def nestedCallTest(self):
        return 19  # Expected value from C++ test - Python does not support nested calls

    def callSecondSide(self):
        return gs_client.callFirstSide()


###############################################################################
# Tests
###############################################################################
def test_FirstSendReceiveInt(client: FirstInterfaceClient):
    for i in range(NUMBER):
        client.firstSendInt(i)

    for i in range(NUMBER - 1, -1, -1):
        b = client.firstReceiveInt()
        assert i == b


def test_FirstSendReceiveInt2(client: FirstInterfaceClient):
    for i in range(NUMBER):
        client.firstSendInt(i)

    for i in range(NUMBER - 1, -1, -1):
        b = client.firstReceiveInt()
        assert i == b


@pytest.mark.skip(reason="Not implemented for python")
def test_NestedCallTest(client: FirstInterfaceClient):
    client.stopSecondSide()

    while not enabled:
        time.sleep(1)

    assert client.nestedCallTest() == NESTED_CALLS_COUNT * 2 - 1


def test_GetResultFromSecondSide(client: FirstInterfaceClient):
    assert client.getResultFromSecondSide() == 0


def test_testCasesAreDone(client: FirstInterfaceClient):
    assert client.testCasesAreDone() is None


def test_is_server_passing(server: int):
    assert server == 0
