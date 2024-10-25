#!/bin/python3

#
# Copyright 2023 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

from erpc.basic_codec import BasicCodec
from erpc.client import ClientManager
from erpc.transport import TCPTransport

import config
from shim import hello_world


def main():
    # load configuration macros from C header file
    cfg = config.lead_config("../config.h")

    # init eRPC client infrastructure
    transport = TCPTransport(cfg["ERPC_HOSTNAME"], int(cfg["ERPC_PORT"]), False)
    client_manager = ClientManager(transport, BasicCodec)

    # init eRPC client
    client = hello_world.client.TextServiceClient(client_manager)

    # do eRPC call
    if client.printText("Hello world!"):
        print("Message received by server.")

    # Stop server
    client.stopServer()


if __name__ == "__main__":
    main()
