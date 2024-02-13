#!/bin/python3
import sys

sys.path.insert(1, '../../../erpc_python')  # nopep8
sys.path.insert(2, '../../')  # nopep8

import erpc
import config
from shim import hello_world


def main():
    # load configuration macros from C header file
    cfg = config.loadConfig("../../config.h")

    # init eRPC client infrastructure
    transport = erpc.transport.TCPTransport(cfg["ERPC_HOSTNAME"], int(cfg["ERPC_PORT"]), False)
    clientManager = erpc.client.ClientManager(transport, erpc.basic_codec.BasicCodec)

    # init eRPC client
    client = hello_world.client.TextServiceClient(clientManager)

    # do eRPC call
    client.printText("Hello world!")

if __name__ == "__main__":
    main()
