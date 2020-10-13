/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_server.h"

#include "assert.h"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

#if ERPC_NESTED_CALLS_DETECTION
bool nestingDetection = false;
#endif

void Server::setTransport(Transport *transport)
{
    m_transport = transport;
}

void Server::addService(Service *service)
{
    if (!m_firstService)
    {
        m_firstService = service;
        return;
    }

    Service *link = m_firstService;
    while (link->getNext() != NULL)
    {
        link = link->getNext();
    }

    link->setNext(service);
}

void Server::removeService(Service *service)
{
    Service *link = m_firstService;

    if (link == service)
    {
        m_firstService = link->getNext();
        return;
    }
    while (link != NULL)
    {
        if (link->getNext() == service)
        {
            link->setNext(link->getNext()->getNext());
            return;
        }
        link = link->getNext();
    }
}

erpc_status_t Server::readHeadOfMessage(Codec *codec, message_type_t &msgType, uint32_t &serviceId, uint32_t &methodId,
                                        uint32_t &sequence)
{
    codec->startReadMessage(&msgType, &serviceId, &methodId, &sequence);
    return codec->getStatus();
}

erpc_status_t Server::processMessage(Codec *codec, message_type_t msgType, uint32_t serviceId, uint32_t methodId,
                                     uint32_t sequence)
{
    if (msgType != kInvocationMessage && msgType != kOnewayMessage)
    {
        return kErpcStatus_InvalidArgument;
    }

    Service *service = findServiceWithId(serviceId);
    if (!service)
    {
        return kErpcStatus_InvalidArgument;
    }

    return service->handleInvocation(methodId, sequence, codec, m_messageFactory);
}

Service *Server::findServiceWithId(uint32_t serviceId)
{
    Service *service = m_firstService;
    while (service != NULL)
    {
        if (service->getServiceId() == serviceId)
        {
            return service;
        }

        service = service->getNext();
    }
    return NULL;
}
