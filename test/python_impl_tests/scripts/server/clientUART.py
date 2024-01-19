#!/usr/bin/python

# Copyright 2022 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

from __future__ import print_function
import os, sys, inspect
from .connection import ConnectionUART, Client
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))))
try:
    import erpc
except ImportError:
    print("Could not import erpc.\r\nPlease install it first by running \"python setup.py install\" in folder \"erpc/erpc_python/\".")
    sys.exit()

###############################################################################
# Client
###############################################################################
    
class ClientUART(ConnectionUART, Client):

    def __init__(self, port, baudrate):
        ConnectionUART.__init__(self, port, baudrate)
        Client.__init__(self)
        # initialize TCP transport layer
        try:
            self.transport = erpc.transport.SerialTransport(self.port, self.baudrate)
        except ConnectionRefusedError:
            print("eRPC server not found.")
            sys.exit()

    ## Run client on specified transport layer
    def runClient(self, constructors):
        print('Client connecting to a host on port %s with baudrate %s bps.' % (self.port, self.baudrate))
        # create test annotations eRPC service
        clientManager = erpc.client.ClientManager(self.transport, erpc.basic_codec.BasicCodec)
        self.initClients(constructors, clientManager)