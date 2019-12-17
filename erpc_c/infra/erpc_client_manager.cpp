/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_client_manager.h"
#include "assert.h"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

#if ERPC_NESTED_CALLS_DETECTION
extern bool nestingDetection;
#pragma weak nestingDetection
bool nestingDetection = false;
#endif

void ClientManager::setTransport(Transport *transport)
{
    m_transport = transport;
}

RequestContext ClientManager::createRequest(bool isOneway)
{
    // Create codec to read and write the request.
    Codec *codec = createBufferAndCodec();

    return RequestContext(++m_sequence, codec, isOneway);
}

void ClientManager::performRequest(RequestContext &request)
{
    // Check the codec status
    if (kErpcStatus_Success != (request.getCodec()->getStatus()))
    {
        // Do not perform the request
        return;
    }

#if ERPC_NESTED_CALLS
    assert(m_serverThreadId && "server thread id was not set");
    if (Thread::getCurrentThreadId() == m_serverThreadId)
    {
        return performNestedClientRequest(request);
    }
#endif
    return performClientRequest(request);
}

void ClientManager::performClientRequest(RequestContext &request)
{
#if ERPC_NESTED_CALLS_DETECTION
    if (!request.isOneway() && nestingDetection)
    {
        request.getCodec()->updateStatus(kErpcStatus_NestedCallFailure);
        return;
    }
#endif

    erpc_status_t err;

#if ERPC_MESSAGE_LOGGING
    err = logMessage(request.getCodec()->getBuffer());
    if (err)
    {
        request.getCodec()->updateStatus(err);
        return;
    }
#endif

    // Send invocation request to server.
    err = m_transport->send(request.getCodec()->getBuffer());
    if (err)
    {
        request.getCodec()->updateStatus(err);
        return;
    }

    // If the request is oneway, then there is nothing more to do.
    if (!request.isOneway())
    {
        // Receive reply.
        err = m_transport->receive(request.getCodec()->getBuffer());
        if (err)
        {
            request.getCodec()->updateStatus(err);
            return;
        }

#if ERPC_MESSAGE_LOGGING
        err = logMessage(request.getCodec()->getBuffer());
        if (err)
        {
            request.getCodec()->updateStatus(err);
            return;
        }
#endif

        // Check the reply.
        err = verifyReply(request);
        if (err)
        {
            request.getCodec()->updateStatus(err);
            return;
        }
    }

    return;
}

#if ERPC_NESTED_CALLS
void ClientManager::performNestedClientRequest(RequestContext &request)
{
    assert(m_transport && "transport/arbitrator not set");

    erpc_status_t err;

#if ERPC_MESSAGE_LOGGING
    err = logMessage(request.getCodec()->getBuffer());
    if (err)
    {
        request.getCodec()->updateStatus(err);
        return;
    }
#endif

    // Send invocation request to server.
    err = m_transport->send(request.getCodec()->getBuffer());
    if (err)
    {
        request.getCodec()->updateStatus(err);
        return;
    }

    // If the request is oneway, then there is nothing more to do.
    if (!request.isOneway())
    {
        // Receive reply.
        assert(m_server && "server for nesting calls was not set");
        err = m_server->run(request);
        if (err)
        {
            request.getCodec()->updateStatus(err);
            return;
        }

#if ERPC_MESSAGE_LOGGING
        err = logMessage(request.getCodec()->getBuffer());
        if (err)
        {
            request.getCodec()->updateStatus(err);
            return;
        }
#endif

        // Check the reply.
        err = verifyReply(request);
        if (err)
        {
            request.getCodec()->updateStatus(err);
            return;
        }
    }
}
#endif

erpc_status_t ClientManager::verifyReply(RequestContext &request)
{
    // Some transport layers change the request's message buffer pointer (for things like zero
    // copy support), so inCodec must be reset to work with correct buffer.
    request.getCodec()->reset();

    // Extract the reply header.
    message_type_t msgType;
    uint32_t service;
    uint32_t requestNumber;
    uint32_t sequence;
    request.getCodec()->startReadMessage(&msgType, &service, &requestNumber, &sequence);
    erpc_status_t err = request.getCodec()->getStatus();
    if (err)
    {
        return err;
    }

    // Verify that this is a reply to the request we just sent.
    if (msgType != kReplyMessage || sequence != request.getSequence())
    {
        return kErpcStatus_ExpectedReply;
    }

    return kErpcStatus_Success;
}

Codec *ClientManager::createBufferAndCodec(void)
{
    Codec *codec = m_codecFactory->create();
    if (!codec)
    {
        return NULL;
    }

    MessageBuffer message = m_messageFactory->create();
    if (!message.get())
    {
        // Dispose of buffers and codecs.
        m_codecFactory->dispose(codec);
        return NULL;
    }

    codec->setBuffer(message);

    return codec;
}

void ClientManager::releaseRequest(RequestContext &request)
{
    m_messageFactory->dispose(request.getCodec()->getBuffer());
    m_codecFactory->dispose(request.getCodec());
}

void ClientManager::callErrorHandler(erpc_status_t err, uint32_t functionID)
{
    if (m_errorHandler != NULL)
    {
        m_errorHandler(err, functionID);
    }
}
