#!/usr/bin/env python

# Copyright 2016-2025 NXP
#
# SPDX-License-Identifier: BSD-3-Clause

from __future__ import print_function

import threading
from typing import Optional, Type

from erpc.codec import Codec
from erpc.transport import Transport, TransportError
from .client import RequestError
from .server import Server


class SimpleServer(Server):
    def __init__(self, transport: Optional[Transport], codecClass: Optional[Type[Codec]]):
        super(SimpleServer, self).__init__(transport, codecClass)
        self._run = True

    def run(self) -> None:
        self._run = True
        while self._run:
            try:
                self._receive_request()
            except (RequestError, TransportError) as e:
                print(f"Error while processing request: {str(e)}")

    def stop(self) -> None:
        self._run = False

    def _receive_request(self):
        assert self.transport is not None, "No Transport is set"
        assert self.codec_class is not None, "No Codec class is set"

        message = self.transport.receive()

        codec = self.codec_class()
        codec.buffer = message

        self._process_request(codec)

        if len(codec.buffer):
            self.transport.send(codec.buffer)


class ServerThread(SimpleServer):
    def __init__(self, transport: Transport, codecClass: Type[Codec]):
        super(ServerThread, self).__init__(transport, codecClass)
        self._thread = threading.Thread(target=self.run, name="erpc_server")
        self._thread.daemon = True

    def start(self):
        self._thread.start()
