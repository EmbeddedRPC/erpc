#!/usr/bin/env python

# Copyright 2016-2025 NXP
# Copyright 2020 ACRIOS Systems s.r.o.
#
# SPDX-License-Identifier: BSD-3-Clause

from typing import Callable, Type, Optional

from erpc.transport import Transport

from .client import RequestError
from .codec import MessageType, Codec


class Service(object):
    def __init__(self, serviceId: int):
        self._id = serviceId
        self._methods: dict[int, Callable[[int, Codec], None]] = {}

    @property
    def service_id(self) -> int:
        return self._id

    def handle_invocation(self, methodId: int, sequence: int, codec: Codec):
        try:
            self._methods[methodId](sequence, codec)
        except Exception as e:
            raise RequestError(f"invalid method ID {methodId} or method implementation: {str(e)}")

class Server(object):
    def __init__(self, transport: Optional[Transport], codecClass: Optional[Type[Codec]]):
        self._transport = transport
        self._codec_class = codecClass
        self._services: list[Service] = []

    @property
    def transport(self) -> Optional[Transport]:
        return self._transport

    @transport.setter
    def transport(self, value: Transport):
        self._transport = value

    @property
    def codec_class(self) -> Optional[Type[Codec]]:
        return self._codec_class

    @codec_class.setter
    def codec_class(self, value: Type[Codec]):
        self._codec_class = value

    @property
    def services(self) -> list["Service"]:
        return self._services

    def get_service_with_id(self, theId: int):
        matches = [x for x in self._services if x.service_id == theId]
        if len(matches):
            return matches[0]
        else:
            return None

    def add_service(self, service: "Service"):
        assert self.get_service_with_id(service.service_id) is None
        self._services.append(service)

    def run(self) -> None:
        raise NotImplementedError()

    def stop(self) -> None:
        pass

    def _process_request(self, codec: Codec):
        info = codec.start_read_message()
        if info.type not in [MessageType.kInvocationMessage, MessageType.kOnewayMessage]:
            raise RequestError("invalid type of incoming request")

        service = self.get_service_with_id(info.service)
        if service is None:
            raise RequestError("invalid service ID (%d)" % info.service)
        service.handle_invocation(info.request, info.sequence, codec)

        if info.type is MessageType.kOnewayMessage:
            codec.reset()
