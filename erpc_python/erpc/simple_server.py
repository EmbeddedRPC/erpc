#!/usr/bin/env python

# Copyright (c) 2015-2016 Freescale Semiconductor, Inc.
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

