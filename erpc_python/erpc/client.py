#!/usr/bin/env python

# Copyright 2016 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

from .codec import MessageType

class RequestError(RuntimeError):
    pass

class ClientManager(object):
    def __init__(self, transport=None, codecClass=None):
        self._transport = transport
        self._arbitrator = None
        self._codecClass = codecClass
        self._sequence = 0

    @property
    def transport(self):
        return self._transport

    @transport.setter
    def transport(self, value):
        self._transport = value

    @property
    def arbitrator(self):
        return self._arbitrator

    @arbitrator.setter
    def arbitrator(self, arb):
        self._arbitrator = arb

    @property
    def codec_class(self):
        return self._codecClass

    @codec_class.setter
    def codec_class(self, value):
        self._codecClass = value

    @property
    def sequence(self):
        self._sequence += 1
        return self._sequence

    def create_request(self, isOneway=False):
        msg = bytearray()
        codec = self.codec_class()
        codec.buffer = msg
        return RequestContext(self.sequence, msg, codec, isOneway)

    def perform_request(self, request):
        # Arbitrate requests.
        token = None
        if self._arbitrator is not None and not request.is_oneway:
            token = self._arbitrator.prepare_client_receive(request)

        # Send serialized request to server.
        self.transport.send(request.codec.buffer)

        if not request.is_oneway:
            if token is not None:
                msg = self._arbitrator.client_receive(token)
            else:
                msg = self.transport.receive()
            request.codec.buffer = msg

            info = request.codec.start_read_message()
            if info.type != MessageType.kReplyMessage:
                raise RequestError("invalid reply message type")
            if info.sequence != request.sequence:
                raise RequestError("unexpected sequence number in reply (was %d, expected %d)"
                            % (info.sequence, request.sequence))


class RequestContext(object):
    def __init__(self, sequence, message, codec, isOneway):
        self._sequence = sequence
        self._message = message
        self._codec = codec
        self._isOneway = isOneway

    @property
    def sequence(self):
        return self._sequence

    @property
    def message(self):
        return self._message

    @property
    def codec(self):
        return self._codec

    @property
    def is_oneway(self):
        return self._isOneway



