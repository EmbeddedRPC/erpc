#!/usr/bin/env python

# Copyright 2016 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import threading

from .codec import MessageType
from .transport import Transport
from .client import RequestContext


class ClientInfo:
    event = None
    msg = None


class TransportArbitrator(Transport):
    """ Shares a transport between a server and multiple clients.

    Args:
        Transport (_type_): Inherit and define transport interface methods
    """

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

    def prepare_client_receive(self, requestContext: RequestContext):
        """ Add a client request to the client list.

        This call is made by the client thread prior to sending the invocation to the server. It
        Ensures that the transport arbitrator has the client's response message buffer ready in
        case it sees the response before the client even has a chance to call client_receive().

        Args:
            requestContext (client.RequestContext): _description_

        Returns:
            int: A token value to be passed to client_receive().
        """

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

    def client_receive(self, token: int):
        """ Receive method for the client.

        Blocks until the a reply message is received with the expected sequence number that is
        associated with @a token. The client must have called prepare_client_receive() previously.

        Args:
            token (int): The token previously returned by prepare_client_receive().

        Returns:
            bytearray: containing the received message.
        """
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
