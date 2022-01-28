/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "erpc_transport_arbitrator.h"

#include "erpc_manually_constructed.h"

#include <cassert>
#include <cstdio>
#include <string>

#if ERPC_THREADS_IS(NONE)
#error "Arbitrator code does not work in no-threading configuration."
#endif

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

ERPC_MANUALLY_CONSTRUCTED_ARRAY_STATIC(TransportArbitrator::PendingClientInfo, s_pendingClientInfoArray,
                                       ERPC_CLIENTS_THREADS_AMOUNT);

TransportArbitrator::TransportArbitrator(void)
: Transport()
, m_sharedTransport(NULL)
, m_codec(NULL)
, m_clientList(NULL)
, m_clientFreeList(NULL)
, m_clientListMutex()
{
}

TransportArbitrator::~TransportArbitrator(void)
{
    // Dispose of client info objects.
    freeClientList(m_clientList);
    freeClientList(m_clientFreeList);
}

void TransportArbitrator::setCrc16(Crc16 *crcImpl)
{
    assert(crcImpl);
    assert(m_sharedTransport);
    m_sharedTransport->setCrc16(crcImpl);
}

bool TransportArbitrator::hasMessage(void)
{
    assert(m_sharedTransport && "shared transport is not set");

    return m_sharedTransport->hasMessage();
}

erpc_status_t TransportArbitrator::receive(MessageBuffer *message)
{
    assert(m_sharedTransport && "shared transport is not set");

    erpc_status_t err;
    message_type_t msgType;
    uint32_t service;
    uint32_t requestNumber;
    uint32_t sequence;
    PendingClientInfo *client;

    while (true)
    {
        // Receive a message.
        err = m_sharedTransport->receive(message);
        if (err != kErpcStatus_Success)
        {
            // if we timeout, we must unblock all pending client(s)
            if (err == kErpcStatus_Timeout)
            {
                client = m_clientList;
                for (; client; client = client->m_next)
                {
                    if (client->m_isValid)
                    {
                        client->m_sem.put();
                    }
                }
            }
            break;
        }

        m_codec->setBuffer(*message);

        // Parse the message header.
        m_codec->startReadMessage(&msgType, &service, &requestNumber, &sequence);
        err = m_codec->getStatus();
        if (err != kErpcStatus_Success)
        {
            continue;
        }

        // If this message is an invocation, return it to the calling server.
        if ((msgType == kInvocationMessage) || (msgType == kOnewayMessage))
        {
            break;
        }

        // Just ignore messages we don't know what to do with.
        if (msgType != kReplyMessage)
        {
            continue;
        }

        // Check if there is a client waiting for this message.
        client = m_clientList;
        for (; client; client = client->m_next)
        {
            if (client->m_isValid && (sequence == client->m_request->getSequence()))
            {
                // Swap the received message buffer with the client's message buffer.
                client->m_request->getCodec()->getBuffer()->swap(message);

                // Wake up the client receive thread.
                client->m_sem.put();
                break;
            }
        }

#if ERPC_NESTED_CALLS
        // If received answer is not for postponed client, it can be for nested server call.
        if (client == NULL)
        {
            break;
        }
#endif
    }

    return err;
}

erpc_status_t TransportArbitrator::send(MessageBuffer *message)
{
    assert(m_sharedTransport && "shared transport is not set");
    return m_sharedTransport->send(message);
}

TransportArbitrator::client_token_t TransportArbitrator::prepareClientReceive(RequestContext &request)
{
    PendingClientInfo *info = addPendingClient();
    if (NULL != info)
    {
        info->m_request = &request;
        info->m_isValid = true;
    }
    return reinterpret_cast<client_token_t>(info);
}

erpc_status_t TransportArbitrator::clientReceive(client_token_t token)
{
    assert((token != 0) && "invalid client token");

    // Convert token to pointer to info struct for this client receive request.
    PendingClientInfo *info = reinterpret_cast<PendingClientInfo *>(token);

    // Wait on the semaphore until we're signaled.
    info->m_sem.get(Semaphore::kWaitForever);

    removePendingClient(info);

    return kErpcStatus_Success;
}

TransportArbitrator::PendingClientInfo *TransportArbitrator::createPendingClient(void){ ERPC_CREATE_NEW_OBJECT(
    TransportArbitrator::PendingClientInfo, s_pendingClientInfoArray, ERPC_CLIENTS_THREADS_AMOUNT) }

TransportArbitrator::PendingClientInfo *TransportArbitrator::addPendingClient(void)
{
    Mutex::Guard lock(m_clientListMutex);

    // Get a free client info node, or allocate one.
    PendingClientInfo *info = NULL;
    if (m_clientFreeList == NULL)
    {
        info = createPendingClient();
    }
    else
    {
        info = m_clientFreeList;
        m_clientFreeList = m_clientFreeList->m_next;
    }

    // Add to active list.
	  info->m_next = m_clientList;
	  m_clientList = info;

    return info;
}

void TransportArbitrator::removePendingClient(PendingClientInfo *info)
{
    Mutex::Guard lock(m_clientListMutex);
    PendingClientInfo *node;

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
        node = m_clientList;
        while (node != NULL)
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
    PendingClientInfo *temp;

    while (info != NULL)
    {
        temp = info;
        info = info->m_next;
        ERPC_DESTROY_OBJECT(temp, s_pendingClientInfoArray, ERPC_CLIENTS_THREADS_AMOUNT)
    }
}

TransportArbitrator::PendingClientInfo::PendingClientInfo(void)
: m_request(NULL)
, m_sem(0)
, m_isValid(false)
, m_next(NULL)
{
}

TransportArbitrator::PendingClientInfo::~PendingClientInfo(void) {}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
