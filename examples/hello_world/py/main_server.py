#!/bin/python3
import sys

sys.path.insert(1, '../../../erpc_python')  # nopep8
sys.path.insert(2, '../../')  # nopep8
sys.path.insert(3, '../')  # nopep8

import erpc
import config
from shim.py import hello_world

class MatrixMultiplyServiceHandler(hello_world.interface.ITextService):
    def printText(self, text):
        '''eRPC call definition'''

        print(text)


def main():
    # load configuration macros from C header file
    cfg = config.loadConfig("../../config.h")

    # init service
    handler = MatrixMultiplyServiceHandler()
    service = hello_world.server.TextServiceService(handler)

    # init eRPC server infrastructure
    transport = erpc.transport.TCPTransport(cfg["ERPC_HOSTNAME"], int(cfg["ERPC_PORT"]), True)
    server = erpc.simple_server.SimpleServer(transport, erpc.basic_codec.BasicCodec)
    server.add_service(service)

    # run server
    try:
        server.run()
    except:
        pass

main()
