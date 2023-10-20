#!/usr/bin/python

# Copyright 2022 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

###############################################################################
# Server
###############################################################################
    
class Server:
    def __init__(self):
        self.serverServices = []

###############################################################################
# Client
###############################################################################
    
class Client:
    def __init__(self):
        self.clientServices = []
            
    def initClients(self, constructors, clientManager):
        for constructor in constructors:
            self.clientServices.append(constructor(clientManager))

    def getClient(self, serviceId):
        for service in self.clientServices:
            if service.SERVICE_ID == serviceId:
                return service
        return None

###############################################################################
# Connection
###############################################################################
    
###############################################################################
# TCP
###############################################################################
    
class ConnectionTCP():
    def __init__(self, host, port):
        self.host = host
        self.port = int(port)

###############################################################################
# UART
###############################################################################
    
class ConnectionUART():
    def __init__(self, port, baudrate):
        self.port = port
        self.baudrate = int(baudrate)
