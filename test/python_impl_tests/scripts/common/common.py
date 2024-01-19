#!/usr/bin/python

# Copyright 2021 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import inspect, os, sys
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe()))))) 
#from service import test_unit_test_common
from service.test_const.erpc_outputs import test_unit_test_common
from server.serverTCP import ServerTCP

###############################################################################
# Variables
###############################################################################


###############################################################################
# Functions
###############################################################################

def runTesttestQuit(client):
    # send request to the server
    print('\r\neRPC request is sent to the server')
    return client.quit()

###############################################################################
# Server
###############################################################################

## Add service handler
class CommonTestServiceHandler(test_unit_test_common.interface.ICommon):
    ## eRPC add method
    # @param a First addend
    # @param b Second addend
    def quit(self):
        print('\r\nquit service reached: \r\n=============')
        sys.stdout.flush()
        ServerTCP.stopServer()
        return
