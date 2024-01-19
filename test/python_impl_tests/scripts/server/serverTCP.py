#!/usr/bin/python

# Copyright 2022 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import os, sys, inspect
from .connection import ConnectionTCP
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))))
try:
    import erpc
except ImportError:
    print("Could not import erpc.\r\nPlease install it first by running \"python setup.py install\" in folder \"erpc/erpc_python/\".")
    sys.exit()

###############################################################################
# Server
###############################################################################

class ServerTCP(ConnectionTCP):
    server=None

    def __init__(self, host, port):
        ConnectionTCP.__init__(self, host, port)
        self.server = True
        # initialize TCP transport layer
        self.transport = erpc.transport.TCPTransport(host, int(port), self.server)

    ## Run server on specified transport layer
    def runServer(self, services):
        # Note: TCP server is ran in daemon thread, so flush is needed to see printouts
        print('Server created on \"%s:%s\"' % (self.host, self.port))
        # create eRPC service for current test
        if services[0] is None:
            print('\r\neRPC service not found.')
            sys.exit()
        # run server
        ServerTCP.server = erpc.simple_server.SimpleServer(self.transport, erpc.basic_codec.BasicCodec)
        for service in services:
            ServerTCP.server.add_service(service)
        print('\r\nWait for client to send a eRPC request')
        sys.stdout.flush()
        try:
            ServerTCP.server.run()
        except:
            print("Server closed!")
    ## Run server on specified transport layer
    def stopServer():
        if ServerTCP.server is not None:
            ServerTCP.server.stop()
    
