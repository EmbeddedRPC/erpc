#!/usr/bin/env python

# Copyright (c) 2015 Freescale Semiconductor, Inc.
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

from .codec import MessageType
from .client import RequestError

class Service(object):
    def __init__(self, serviceId):
        self._id = serviceId
        self._methods = {}

    @property
    def service_id(self):
        return self._id

    def handle_invocation(self, methodId, sequence, codec):
        try:
            self._methods[methodId](sequence, codec)
        except KeyError:
            raise RequestError("invalid method ID (%d)" % (methodId))

class Server(object):
    def __init__(self, transport=None, codecClass=None):
        self._transport = transport
        self._codecClass = codecClass
        self._services = []

    @property
    def transport(self):
        return self._transport

    @transport.setter
    def transport(self, value):
        self._transport = value

    @property
    def codec_class(self):
        return self._codecClass

    @codec_class.setter
    def codec_class(self, value):
        self._codecClass = value

    @property
    def services(self):
        return self._services

    def get_service_with_id(self, theId):
        matches = [x for x in self._services if x.service_id == theId]
        if len(matches):
            return matches[0]
        else:
            return None

    def add_service(self, service):
        assert self.get_service_with_id(service.service_id) is None
        self._services.append(service)

    def run(self):
        raise NotImplementedError()

    def stop(self):
        pass

    def _process_request(self, codec):
        info = codec.start_read_message()
        if info.type not in [MessageType.kInvocationMessage, MessageType.kOnewayMessage]:
            raise RequestError("invalid type of incoming request")

        service = self.get_service_with_id(info.service)
        if service is None:
            raise RequestError("invalid service ID (%d)" % info.service)
        service.handle_invocation(info.request, info.sequence, codec)

        if info.type is MessageType.kOnewayMessage:
            codec.reset()
