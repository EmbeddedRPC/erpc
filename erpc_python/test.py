#! /usr/bin/env python

from __future__ import print_function

import sys
import math
import random
import erpc

import erpc_test

SERVER_PORT = 12345

def server():
    class MathServiceHandler(erpc_test.interface.IArithmetic):
        def add(self, a, b):
            return a + b

        def sub(self, a, b):
            return a - b

    class ServerServiceHandler(erpc_test.interface.IServer):
        def quit(self):
            return exit(0)

    transport = erpc.transport.TCPTransport('localhost', SERVER_PORT, True)
    server = erpc.simple_server.SimpleServer(transport, erpc.basic_codec.BasicCodec)
    mathHandler = MathServiceHandler()
    quitHandler = ServerServiceHandler()
    mathService = erpc_test.server.ArithmeticService(mathHandler)
    quitService = erpc_test.server.ServerService(quitHandler)
    server.add_service(mathService)
    server.add_service(quitService)
    server.run()

def client():
    transport = erpc.transport.TCPTransport('localhost', SERVER_PORT, False)
    clientManager = erpc.client.ClientManager(transport, erpc.basic_codec.BasicCodec)

    myClient = erpc_test.client.ArithmeticClient(clientManager)

    for z in range(1, 11):
        a, b = random.randint(1, 65535), random.randint(1, 65535)
        x = myClient.add(a, b)
        print("add(%f, %f)==%f" % (a, b, x))

        a, b = x, random.randint(1, 65535)
        x = myClient.sub(a, b)
        print("sub(%f, %f)==%f" % (a, b, x))

    try:
        quitClient = erpc_test.client.ServerClient(clientManager)
        quitClient.quit()
    except erpc.transport.ConnectionClosed:
        print("Connected closed from server side")

if __name__ == "__main__":
    if sys.argv[1] == '-c':
        client()
    elif sys.argv[1] == '-s':
        server()

