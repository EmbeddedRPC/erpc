#!/usr/bin/env python

# Copyright 2016-2025 NXP
#
# SPDX-License-Identifier: BSD-3-Clause

from typing import Optional, Type, TYPE_CHECKING

if TYPE_CHECKING:
    from .arbitrator import TransportArbitrator
    from .transport import Transport

from .codec import Codec, MessageType

class RequestError(RuntimeError):
    pass


class ClientManager(object):
    def __init__(self, transport: Optional["Transport"] = None, codecClass: Optional[Type[Codec]] = None):
        self._transport = transport
        self._arbitrator: Optional["TransportArbitrator"] = None
        self._codec_class = codecClass
        self._sequence = 0

    @property
    def transport(self) -> Optional["Transport"]:
        return self._transport

    @transport.setter
    def transport(self, value: "Transport"):
        self._transport = value

    @property
    def arbitrator(self) -> Optional["TransportArbitrator"]:
        return self._arbitrator

    @arbitrator.setter
    def arbitrator(self, arb: "TransportArbitrator"):
        self._arbitrator = arb

    @property
    def codec_class(self) -> Optional[Type[Codec]]:
        return self._codec_class

    @codec_class.setter
    def codec_class(self, value: Type[Codec]):
        self._codec_class = value

    @property
    def sequence(self) -> int:
        self._sequence += 1
        return self._sequence

    def create_request(self, isOneway: bool = False) -> "RequestContext":
        assert self.codec_class is not None, "No codec class was set"
        message = bytearray()
        codec = self.codec_class()
        codec.buffer = message
        return RequestContext(self.sequence, message, codec, isOneway)

    def perform_request(self, request: "RequestContext"):
        assert self.transport is not None, "No Transport was set"

        # Arbitrate requests.
        token: Optional[int] = None
        if self._arbitrator is not None and not request.is_oneway:
            token = self._arbitrator.prepare_client_receive(request)

        # Send serialized request to server.
        self.transport.send(request.codec.buffer)

        if not request.is_oneway:
            if token is not None:
                assert self._arbitrator is not None  # Arbitrator can not be none if toke exist
                message = self._arbitrator.client_receive(token)
            else:
                message = self.transport.receive()
            request.codec.buffer = message

            info = request.codec.start_read_message()
            if info.type != MessageType.kReplyMessage:
                raise RequestError("invalid reply message type")
            if info.sequence != request.sequence:
                raise RequestError("unexpected sequence number in reply (was %d, expected %d)"
                                   % (info.sequence, request.sequence))


class RequestContext(object):
    def __init__(self, sequence: int, message: bytes, codec: Codec, isOneway: bool):
        self._sequence = sequence
        self._message = message
        self._codec = codec
        self._is_oneway = isOneway

    @property
    def sequence(self) -> int:
        return self._sequence

    @property
    def message(self) -> bytes:
        return self._message

    @property
    def codec(self) -> Codec:
        return self._codec

    @property
    def is_oneway(self) -> bool:
        return self._is_oneway
