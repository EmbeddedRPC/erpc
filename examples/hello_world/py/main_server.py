#!/bin/python3

#
# Copyright 2023 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

from typing import Optional

from erpc.basic_codec import BasicCodec
from erpc.simple_server import SimpleServer
from erpc.transport import TCPTransport

import config
from shim import hello_world

g_server: Optional[SimpleServer] = None


class MatrixMultiplyServiceHandler(hello_world.interface.ITextService):
    """ eRPC call definition"""

    def printText(self, text):
        print(text)
        return True

    def stopServer(self):
        if g_server:
            g_server.stop()


def main():
    global g_server

    # load configuration macros from C header file
    cfg = config.lead_config("../config.h")

    # init service
    handler = MatrixMultiplyServiceHandler()
    service = hello_world.server.TextServiceService(handler)

    # init eRPC server infrastructure
    transport = TCPTransport(cfg["ERPC_HOSTNAME"], int(cfg["ERPC_PORT"]), True)
    g_server = SimpleServer(transport, BasicCodec)
    g_server.add_service(service)

    print("Starting server.")

    # run server
    g_server.run()

    print("Server stopped.")


if __name__ == "__main__":
    main()
