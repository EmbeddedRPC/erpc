#!/usr/bin/env python

# Copyright 2016-2025 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import threading
import dataclasses
from typing import Optional, TYPE_CHECKING

from .codec import MessageType, Codec
from .transport import Transport
from .client import RequestError

if TYPE_CHECKING:
    from .client import RequestContext


@dataclasses.dataclass
class ClientInfo:
    event: threading.Event
    message: Optional[bytearray] = None


class TransportArbitrator(Transport):
    """ Shares transport between a server and multiple clients.

    Args:
        Transport (_type_): Inherit and define transport interface methods
    """

    def __init__(self, sharedTransport: Optional[Transport] = None, codec: Optional[Codec] = None):
        super().__init__()
        self._transport = sharedTransport
        self._codec = codec
        self._pending_clients: dict[int, ClientInfo] = {}
        self._lock = threading.Lock()

    @property
    def shared_transport(self) -> Optional[Transport]:
        return self._transport

    @shared_transport.setter
    def shared_transport(self, transport: Transport):
        self._transport = transport

    @property
    def codec(self) -> Optional[Codec]:
        return self._codec

    @codec.setter
    def codec(self, theCodec: Codec):
        self._codec = theCodec

    def send(self, message: bytes):
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
                client.message = msg
                client.event.set()
            except KeyError:
                # No client was found, unexpected sequence number!
                pass

    def prepare_client_receive(self, requestContext: "RequestContext"):
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
        info = ClientInfo(threading.Event())

        # Add this client to the pending clients' dict.
        try:
            self._lock.acquire()
            self._pending_clients[requestContext.sequence] = info
        finally:
            self._lock.release()

        return requestContext.sequence

    def client_receive(self, token: int) -> bytearray:
        """ Receive method for the client.

        Blocks until the reply message is received with the expected sequence number that is
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

            # Remove this client from the pending clients' dict.
            try:
                self._lock.acquire()
                del self._pending_clients[token]
            finally:
                self._lock.release()

            # Return the received message.
            assert client.message # Message should be available at this moment
            return client.message
        except KeyError:
            raise RequestError(f"Pending client with token '{token}' not found")
