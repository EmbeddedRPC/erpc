#!/usr/bin/python

# Copyright 2022 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import sys, argparse
from .serverTCP import ServerTCP
from .clientTCP import ClientTCP
from .clientUART import ClientUART
from .serverUART import ServerUART
from subprocess import Popen

class Runner:

    def __init__(self, name, params, services, constructors):
        self.services = services if isinstance(services, list) else [services]     
        self.constructors = constructors if isinstance(constructors, list) else [constructors]
        self.client = None
        self.proc = None
        self.argParser = self.initParser()

        if '--twister-harness' in sys.argv:
            # Filter all unnecessary arguments from Twister
            args = [item for item in sys.argv if ('--device-serial' in item or '--device-serial-baud' in item)]
            args.append('-c')
            self.args = self.argParser.parse_args(args)
        else:
            try:
                self.args = self.argParser.parse_args()
            except:
                args = [item for item in sys.argv if (item in ['-c', '-s', '--server', '--client']) or ('=' in item)] 
                self.args = self.argParser.parse_args(args)
            self.initTest(name, params)

    def initParser(self):
        # parse cmd parameters
        argParser = argparse.ArgumentParser(description='eRPC Matrix Multiply example')
        argParser.add_argument('-c', '--client', action='store_true', help='Run client')
        argParser.add_argument('-s', '--server', action='store_true', help='Run server')
        argParser.add_argument('-t', '--host', default='localhost', help='Host IP address (default value is localhost)')
        argParser.add_argument('-p', '--port', default='12345', help='Port (default value is 12345)')
        argParser.add_argument('-S', '--serial', default=None, help='Serial device (default value is None)')
        argParser.add_argument('-B', '--baud', default='115200', help='Baud (default value is 115200)')
        # Zephyr Twister arguments
        argParser.add_argument('--device-serial', default=None, dest='serial')
        argParser.add_argument('--device-serial-baud', default='115200', dest='baud', )
        return argParser
        
    def initTest(self, name, params):
        # check if either server or client has been selected
        if not self.args.client and not self.args.server:
            self.argParser.print_help()
            sys.exit('\r\nEither server or client has to be selected to run')
        if self.args.client and self.args.server:
            print('eRPC ' + name + ' server')
            cmd = ['python'] + params + ["-t", self.args.host, "-p", self.args.port]
            self.proc = Popen(cmd)
        if self.args.client:
            print('eRPC ' + name + ' client')
            #wait a while for server start
            if self.args.server:
                import time
                time.sleep(1)
            self.client = ClientUART(self.args.serial, int(self.args.baud)) if self.args.serial else ClientTCP(self.args.host, self.args.port)
            self.client.runClient(self.constructors)
        elif self.args.server and self.services != None and self.proc is None:
            print('eRPC ' + name + ' server run')
            tcp_server = ServerUART(self.args.serial, int(self.args.baud)) if self.args.serial else ServerTCP(self.args.host, self.args.port)
            tcp_server.runServer(self.services)
    
    def runTest(self, clientIdx, *funcs):
        if self.client is None:
            return
        for items in funcs:
            items(self.client.getClient(clientIdx))
