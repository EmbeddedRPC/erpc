/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_server.hpp"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

#if ERPC_NESTED_CALLS_DETECTION
extern bool nestingDetection;
bool nestingDetection = false;
#endif

void Server::setTransport(Transport *transport)
{
    m_transport = transport;
}

void Server::addService(Service *service)
{
    if (m_firstService == NULL)
    {
        m_firstService = service;
    }
    else
    {

        Service *link = m_firstService;
        while (link->getNext() != NULL)
        {
            link = link->getNext();
        }

        link->setNext(service);
    }
}

void Server::removeService(Service *service)
{
    Service *link = m_firstService;

    if (link == service)
    {
        m_firstService = link->getNext();
    }
    else
    {
        while (link != NULL)
        {
            if (link->getNext() == service)
            {
                link->setNext(link->getNext()->getNext());
                break;
            }
            link = link->getNext();
        }
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
    erpc_status_t err = kErpcStatus_Success;
    Service *service;

    if ((msgType != kInvocationMessage) && (msgType != kOnewayMessage))
    {
        err = kErpcStatus_InvalidArgument;
    }

    if (err == kErpcStatus_Success)
    {
        service = findServiceWithId(serviceId);
        if (service == NULL)
        {
            err = kErpcStatus_InvalidArgument;
        }
    }

    if (err == kErpcStatus_Success)
    {
        err = service->handleInvocation(methodId, sequence, codec, m_messageFactory);
    }

    return err;
}

Service *Server::findServiceWithId(uint32_t serviceId)
{
    Service *service = m_firstService;
    while (service != NULL)
    {
        if (service->getServiceId() == serviceId)
        {
            break;
        }

        service = service->getNext();
    }
    return service;
}
