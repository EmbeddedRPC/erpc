#!/usr/bin/env python

# Copyright (c) 2015-2016 Freescale Semiconductor, Inc.
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
# o Neither the name of Freescale Semiconductor, Inc. nor the names of its
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
        outCodec = self.codec_class()
        outCodec.buffer = msg
        if not isOneway:
            inCodec = self.codec_class()
        return RequestContext(self.sequence, msg, inCodec, outCodec, isOneway)

    def perform_request(self, request):
        # Arbitrate requests.
        token = None
        if self._arbitrator is not None and not request.is_oneway:
            token = self._arbitrator.prepare_client_receive(request)

        # Send serialized request to server.
        self.transport.send(request.out_codec.buffer)

        if not request.is_oneway:
            if token is not None:
                msg = self._arbitrator.client_receive(token)
            else:
                msg = self.transport.receive()
            request.in_codec.buffer = msg
            request.in_codec.reset()

            info = request.in_codec.start_read_message()
            if info.type != MessageType.kReplyMessage:
                raise RequestError("invalid reply message type")
            if info.sequence != request.sequence:
                raise RequestError("unexpected sequence number in reply (was %d, expected %d)"
                            % (info.sequence, request.sequence))


class RequestContext(object):
    def __init__(self, sequence, message, inCodec, outCodec, isOneway):
        self._sequence = sequence
        self._message = message
        self._inCodec = inCodec
        self._outCodec = outCodec
        self._isOneway = isOneway

    @property
    def sequence(self):
        return self._sequence

    @property
    def message(self):
        return self._message

    @property
    def in_codec(self):
        return self._inCodec

    @property
    def out_codec(self):
        return self._outCodec

    @property
    def is_oneway(self):
        return self._isOneway



