/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "transport_arbitrator.h"
#include <cassert>
#include <cstdio>
#include <string>

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

TransportArbitrator::TransportArbitrator()
: Transport()
, m_sharedTransport(NULL)
, m_codec(NULL)
, m_clientList(NULL)
, m_clientFreeList(NULL)
, m_clientListMutex()
{
}

TransportArbitrator::~TransportArbitrator()
{
    // Dispose of client info objects.
    freeClientList(m_clientList);
    freeClientList(m_clientFreeList);
}

erpc_status_t TransportArbitrator::receive(MessageBuffer *message)
{
    assert(m_sharedTransport && "shared transport is not set");

    while (true)
    {
        // Receive a message.
        erpc_status_t err = m_sharedTransport->receive(message);
        if (err)
        {
            return err;
        }

        m_codec->setBuffer(*message);

        // Parse the message header.
        message_type_t msgType;
        uint32_t service;
        uint32_t requestNumber;
        uint32_t sequence;
        err = m_codec->startReadMessage(&msgType, &service, &requestNumber, &sequence);
        if (err)
        {
            continue;
        }

        // If this message is an invocation, return it to the calling server.
        if (msgType == kInvocationMessage || msgType == kOnewayMessage)
        {
            return kErpcStatus_Success;
        }

        // Just ignore messages we don't know what to do with.
        if (msgType != kReplyMessage)
        {
            continue;
        }

        // Check if there is a client waiting for this message.
        PendingClientInfo *client = m_clientList;
        for (; client; client = client->m_next)
        {
            if (client->m_isValid && sequence == client->m_request->getSequence())
            {
                // Swap the received message buffer with the client's message buffer.
                client->m_request->getCodec()->getBuffer()->swap(message);

                // Wake up the client receive thread.
                client->m_sem.put();
                break;
            }
        }
    }
}

erpc_status_t TransportArbitrator::send(MessageBuffer *message)
{
    assert(m_sharedTransport && "shared transport is not set");
    return m_sharedTransport->send(message);
}

TransportArbitrator::client_token_t TransportArbitrator::prepareClientReceive(RequestContext &request)
{
    PendingClientInfo *info = addPendingClient();
    if (!info)
    {
        return 0;
    }
    info->m_request = &request;
    info->m_isValid = true;
    return reinterpret_cast<client_token_t>(info);
}

erpc_status_t TransportArbitrator::clientReceive(client_token_t token)
{
    assert(token != 0 && "invalid client token");

    // Convert token to pointer to info struct for this client receive request.
    PendingClientInfo *info = reinterpret_cast<PendingClientInfo *>(token);

    // Wait on the semaphore until we're signaled.
    info->m_sem.get(Semaphore::kWaitForever);

    removePendingClient(info);

    return kErpcStatus_Success;
}

TransportArbitrator::PendingClientInfo *TransportArbitrator::addPendingClient()
{
    Mutex::Guard lock(m_clientListMutex);

    // Get a free client info node, or allocate one.
    PendingClientInfo *info = NULL;
    if (!m_clientFreeList)
    {
        info = new PendingClientInfo();
    }
    else
    {
        info = m_clientFreeList;
        m_clientFreeList = m_clientFreeList->m_next;
    }

    // Add to active list.
    if (!m_clientList)
    {
        m_clientList = info;
    }
    else
    {
        info->m_next = m_clientList;
        m_clientList = info;
    }

    return info;
}

void TransportArbitrator::removePendingClient(PendingClientInfo *info)
{
    Mutex::Guard lock(m_clientListMutex);

    // Clear fields.
    info->m_request = NULL;
    info->m_isValid = false;

    // Remove from active list.
    if (m_clientList == info)
    {
        m_clientList = info->m_next;
    }
    else
    {
        PendingClientInfo *node = m_clientList;
        while (node)
        {
            if (node->m_next == info)
            {
                node->m_next = info->m_next;
                break;
            }
            node = node->m_next;
        }
    }

    // Add to free list.
    info->m_next = m_clientFreeList;
    m_clientFreeList = info;
}

void TransportArbitrator::freeClientList(PendingClientInfo *list)
{
    PendingClientInfo *info = list;
    while (info)
    {
        PendingClientInfo *temp = info;
        info = info->m_next;
        delete temp;
    }
}

TransportArbitrator::PendingClientInfo::PendingClientInfo()
: m_request(NULL)
, m_sem(0)
, m_isValid(false)
, m_next(NULL)
{
}

TransportArbitrator::PendingClientInfo::~PendingClientInfo()
{
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
