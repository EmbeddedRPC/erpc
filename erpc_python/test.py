#! /usr/bin/env python

# Copyright (c) 2016 Freescale Semiconductor, Inc.
# Copyright 2016 NXP
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# o Redistributions of source code must retain the above copyright notice, this list
#   of conditions and the following disclaimer.
#
# o Redistributions in binary form must reproduce the above copyright notice, this
#   list of conditions and the following disclaimer in the documentation and/or
#   other materials provided with the distribution.
#
# o Neither the name of the copyright holder nor the names of its
#   contributors may be used to endorse or promote products derived from this
#   software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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

