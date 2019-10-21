#!/usr/bin/env python

# Copyright 2016 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

from __future__ import print_function

import threading
from .server import (Service, Server)
from .client import RequestError

class SimpleServer(Server):
    def __init__(self, transport=None, codecClass=None):
        super(SimpleServer, self).__init__(transport, codecClass)
        self._run = True

    def run(self):
        self._run = True
        while self._run:
            try:
                self._receive_request()
            except RequestError as e:
                print("Error while processing request: %s" % (e))

    def stop(self):
        self._run = False

    def _receive_request(self):
        msg = self.transport.receive()
        
        codec = self.codec_class()
        codec.buffer = msg
        
        self._process_request(codec)

        if len(codec.buffer):
            self.transport.send(codec.buffer)

class ServerThread(SimpleServer):
    def __init__(self, transport, codecClass):
        super(ServerThread, self).__init__(transport, codecClass)
        self._thread = threading.Thread(target=self.run, name="erpc_server")
        self._thread.daemon = True

    def start(self):
        self._thread.start()

