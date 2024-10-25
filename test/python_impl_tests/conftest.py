#!/usr/bin/python

# Copyright 2021-2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause

from __future__ import annotations

import multiprocessing
import time
from typing import Optional, Generator, cast

import pytest
from _pytest.config import Config, Notset
from _pytest.config.argparsing import Parser
from _pytest.fixtures import FixtureRequest
from erpc.arbitrator import TransportArbitrator
from erpc.basic_codec import BasicCodec
from erpc.client import ClientManager
from erpc.server import Service
from erpc.simple_server import SimpleServer
from erpc.transport import TCPTransport, SerialTransport

from .test_const.service.erpc_outputs import test_unit_test_common
from .test_const.service.erpc_outputs.test_unit_test_common.client import CommonClient
from .test_const.service.erpc_outputs.test_unit_test_common.server import CommonService


def pytest_addoption(parser: Parser):
    parser.addoption("--client", action="store_true", default=False, help="run all combinations")
    parser.addoption("--server", action="store_true", default=False, help="run all combinations")
    parser.addoption("--host", action="store", default='localhost', help="run all combinations")
    parser.addoption("--port", action="store", default='40', help="run all combinations")
    parser.addoption("--serial", action="store", default=None, help="run all combinations")
    parser.addoption("--baud", action="store", default='115200', help="run all combinations")


###############################################################################
# Classes
###############################################################################
global_server: SimpleServer | None = None


class TransportFactory:
    """
    Build a concreate transport layer based on pytestconfig.

    This class can be passed to new process and then build new transport layer (concreate transport layer cannot be
    passed to new process because of thread lock, so we have to pass it this way).
    """

    def __init__(self, config: Config):
        self.port = int(cast(int, config.getoption("port")))
        self.host = config.getoption("host")
        self.serial = config.getoption("serial")
        self.baud = int(cast(int, config.getoption("baud")))
        self.arbitrator = None

    def build_transport(self, is_server: bool,
                        create_arbitrator: bool = False) -> TCPTransport | SerialTransport | TransportArbitrator:
        if self.serial:
            transport = SerialTransport(self.serial, self.baud)
        else:
            transport = TCPTransport(self.host, self.port, is_server)

        if create_arbitrator:
            transport = TransportArbitrator(transport, BasicCodec())

        return transport


class CommonTestServiceHandler(test_unit_test_common.interface.ICommon):
    """
    Common handler to stop and quit the server
    """

    def quit(self):
        global global_server
        if global_server:
            global_server.stop()

    def getServerAllocated(self):
        return 0


###############################################################################
# Fixtures
###############################################################################

@pytest.fixture(scope="session")
def config(request: FixtureRequest, pytestconfig: Config) -> Generator[Config, None, None]:
    device_object = None

    if pytestconfig.getoption("twister_harness", default=False):
        device_object = request.getfixturevalue('device_object')
        device_object.launch()
        device_object.disconnect()
        time.sleep(1)
        pytestconfig.option.serial = pytestconfig.getoption("device_serial")
        pytestconfig.option.baud = pytestconfig.getoption("device_serial_baud")
        pytestconfig.option.client = True
        pytestconfig.option.server = False

    yield pytestconfig

    if device_object is not None:
        device_object.close()


@pytest.fixture(scope="module")
def transport_factory(config: Config):
    """
    Fixture for transport factory
    :param config: pytest config
    :return: new transport factory
    """
    return TransportFactory(config)


@pytest.fixture(scope="module")
def client_manager(config: Config, transport_factory: TransportFactory, server: SimpleServer) -> ClientManager:
    """
    Fixture for client manager, that is used by all tests for creating client.
    :param config: pytestconfig
    :param transport_factory: TransportFactory to use
    :param server: Ensure that server is created before client
    :return: new client manager
    """
    if not config.getoption("client"):
        pytest.skip("No test's client, skipping.")
    else:
        time.sleep(0.1)  # Add delay to allow server run
        return ClientManager(transport_factory.build_transport(False), BasicCodec)


@pytest.fixture(scope="module")
def common_client(client_manager: ClientManager) -> CommonClient:
    """
    This client is used in all tests to stop the server
    :param client_manager:
    :return: new CommonClient
    """
    return CommonClient(client_manager)


def run_server(transport_factory: TransportFactory, services: list[Service]) -> SimpleServer:
    """
    Auxiliary function, that create SimpleServer using TransportFactory, add CommonService and services specified by
    current test and run it.

    Created server is assigned to global_server variable.
    :param transport_factory: TransportFactory to use
    :param services: list of services defined by current test
    :return: new server
    """
    global global_server

    global_server = SimpleServer(transport_factory.build_transport(True), BasicCodec)

    global_server.add_service(CommonService(CommonTestServiceHandler()))

    for service in services:
        global_server.add_service(service)

    global_server.run()
    print("Server: started")

    return global_server


@pytest.fixture(scope="module", autouse=True)
def server(config: Config,
           transport_factory: TransportFactory,
           services: list[Service]) -> Generator[SimpleServer | None, None, None]:
    """
    Create server if necessary. If server and client should run together, spawn new process with Server.
    :param config: pytestconfig
    :param transport_factory: Transport factory to use or pass to server (new process)
    :param services: list of services defined by current test
    :return: new server or None if executed in new process
    """
    global global_server
    p: Optional[multiprocessing.Process] = None

    # We don't need the server
    if not config.getoption("server"):
        yield
        return

    if config.getoption("server") and config.getoption("client"):
        # Create server in new process
        p = multiprocessing.Process(
            target=run_server,
            args=(transport_factory, services,)
        )
        p.start()
    else:
        run_server(transport_factory, services)

    yield global_server

    if config.getoption("server") and config.getoption("client"):
        assert p is not None
        # Wait for server's process to stop
        p.join()

    print("Server: stopped")
