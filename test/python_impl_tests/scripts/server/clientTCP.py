#!/usr/bin/python

# Copyright 2022 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

from __future__ import print_function
import os, sys, inspect
from .connection import ConnectionTCP, Client
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))))
try:
    import erpc
except ImportError:
    print("Could not import erpc.\r\nPlease install it first by running \"python setup.py install\" in folder \"erpc/erpc_python/\".")
    sys.exit()

###############################################################################
# Client
###############################################################################
    
class ClientTCP(ConnectionTCP, Client):

    def __init__(self, host, port):
        ConnectionTCP.__init__(self, host, port)
        Client.__init__(self)
        self.server = False
        # initialize TCP transport layer
        try:
            self.transport = erpc.transport.TCPTransport(host, int(port), self.server)
        except ConnectionRefusedError:
            print("eRPC server not found.")
            sys.exit()

    ## Run client on specified transport layer
    def runClient(self, constructors):
        print('Client connecting to a host on \"%s:%s\".' % (self.host, self.port))
        # create test eRPC service
        clientManager = erpc.client.ClientManager(self.transport, erpc.basic_codec.BasicCodec)
        self.initClients(constructors, clientManager)
