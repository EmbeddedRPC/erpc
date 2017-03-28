#!/usr/bin/env python

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

import threading
from collections import namedtuple
from .codec import MessageType
from .transport import Transport

class ClientInfo:
    event = None
    msg = None

##
# @brief Shares a transport between a server and multiple clients.
class TransportArbitrator(Transport):
    def __init__(self, sharedTransport=None, codec=None):
        self._transport = sharedTransport
        self._codec = codec
        self._pending_clients = {}
        self._lock = threading.Lock()

    @property
    def shared_transport(self):
        return self._transport

    @shared_transport.setter
    def shared_transport(self, transport):
        self._transport = transport

    @property
    def codec(self):
        return self._codec

    @codec.setter
    def codec(self, theCodec):
        self._codec = theCodec

    def send(self, message):
        assert self._transport is not None, "No shared transport was set"
        self._transport.send(message)

    def receive(self):
        assert self._transport is not None, "No shared transport was set"
        assert self._codec is not None, "No codec was set"

        # Repeatedly receive until we get an invocation request.
        while True:
            # Receive a message from the shared transport.
            msg = self._transport.receive()

            # Parse the message header.
            self._codec.buffer = msg
            info = self._codec.start_read_message()

            # If it's an invocation or oneway, return it to the server.
            if info.type in (MessageType.kInvocationMessage, MessageType.kOnewayMessage):
                return msg
            # Ignore unexpected message types.
            elif info.type != MessageType.kReplyMessage:
                continue

            # Look up the client waiting for this reply.
            try:
                try:
                    self._lock.acquire()
                    client = self._pending_clients[info.sequence]
                finally:
                    self._lock.release()
                client.msg = msg
                client.event.set()
            except KeyError:
                # No client was found, unexpected sequence number!
                pass

    ##
    # @brief Add a client request to the client list.
    #
    # This call is made by the client thread prior to sending the invocation to the server. It
    # ensures that the transport arbitrator has the client's response message buffer ready in
    # case it sees the response before the client even has a chance to call client_receive().
    #
    # @param self
    # @param requestContext
    # @return A token value to be passed to client_receive().
    def prepare_client_receive(self, requestContext):
        # Create pending client info.
        info = ClientInfo()
        info.event = threading.Event()

        # Add this client to the pending clients dict.
        try:
            self._lock.acquire()
            self._pending_clients[requestContext.sequence] = info
        finally:
            self._lock.release()

        return requestContext.sequence

    ##
    # @brief Receive method for the client.
    #
    # Blocks until the a reply message is received with the expected sequence number that is
    # associated with @a token. The client must have called prepare_client_receive() previously.
    #
    # @param self
    # @param token The token previously returned by prepare_client_receive().
    # @return bytearray containing the received message.
    def client_receive(self, token):
        try:
            # Look up our client info.
            try:
                self._lock.acquire()
                client = self._pending_clients[token]
            finally:
                self._lock.release()

            # Wait for the reply to be received.
            client.event.wait()

            # Remove this client from the pending clients dict.
            try:
                self._lock.acquire()
                del self._pending_clients[token]
            finally:
                self._lock.release()

            # Return the received message.
            return client.msg
        except KeyError:
            pass



