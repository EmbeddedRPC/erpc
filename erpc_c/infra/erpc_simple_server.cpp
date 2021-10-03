/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * Copyright 2019-2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_simple_server.h"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

void SimpleServer::disposeBufferAndCodec(Codec *codec)
{
    if (codec != NULL)
    {
        if (codec->getBuffer() != NULL)
        {
            m_messageFactory->dispose(codec->getBuffer());
        }
        m_codecFactory->dispose(codec);
    }
}

erpc_status_t SimpleServer::runInternal(void)
{
    MessageBuffer buff;
    Codec *codec = NULL;

    // Handle the request.
    message_type_t msgType;
    uint32_t serviceId;
    Md5Hash methodId;
    uint32_t sequence;

    erpc_status_t err = runInternalBegin(&codec, buff, msgType, serviceId, methodId, sequence);
    if (err == kErpcStatus_Success)
    {
        err = runInternalEnd(codec, msgType, serviceId, methodId, sequence);
    }

    return err;
}

erpc_status_t SimpleServer::runInternalBegin(Codec **codec, MessageBuffer &buff, message_type_t &msgType,
                                             uint32_t &serviceId, Md5Hash methodId, uint32_t &sequence)
{
    erpc_status_t err = kErpcStatus_Success;

    if (m_messageFactory->createServerBuffer() == true)
    {
        buff = m_messageFactory->create();
        if (!buff.get())
        {
            err = kErpcStatus_MemoryError;
        }
    }

    // Receive the next invocation request.
    if (err == kErpcStatus_Success)
    {
        err = m_transport->receive(&buff);
    }

#if ERPC_PRE_POST_ACTION
    pre_post_action_cb preCB = this->getPreCB();
    if (preCB != NULL)
    {
        preCB();
    }
#endif

#if ERPC_MESSAGE_LOGGING
    if (err == kErpcStatus_Success)
    {
        err = logMessage(&buff);
    }
#endif

    if (err == kErpcStatus_Success)
    {
        *codec = m_codecFactory->create();
        if (*codec == NULL)
        {
            err = kErpcStatus_MemoryError;
        }
    }

    if (err != kErpcStatus_Success)
    {
        // Dispose of buffers.
        if (buff.get() != NULL)
        {
            m_messageFactory->dispose(&buff);
        }
    }

    if (err == kErpcStatus_Success)
    {
        (*codec)->setBuffer(buff);

        err = readHeadOfMessage(*codec, msgType, serviceId, methodId, sequence);
        if (err != kErpcStatus_Success)
        {
            // Dispose of buffers and codecs.
            disposeBufferAndCodec(*codec);
        }
    }

    return err;
}

erpc_status_t SimpleServer::runInternalEnd(Codec *codec, message_type_t msgType, uint32_t serviceId, Md5Hash methodId,
                                           uint32_t sequence)
{
    erpc_status_t err = processMessage(codec, msgType, serviceId, methodId, sequence);

    if (err == kErpcStatus_Success)
    {
        if (msgType != kOnewayMessage)
        {
#if ERPC_MESSAGE_LOGGING
            err = logMessage(codec->getBuffer());
            if (err == kErpcStatus_Success)
            {
#endif
                err = m_transport->send(codec->getBuffer());
#if ERPC_MESSAGE_LOGGING
            }
#endif
        }

#if ERPC_PRE_POST_ACTION
        pre_post_action_cb postCB = this->getPostCB();
        if (postCB != NULL)
        {
            postCB();
        }
#endif
    }

    // Dispose of buffers and codecs.
    disposeBufferAndCodec(codec);

    return err;
}

erpc_status_t SimpleServer::run(void)
{
    erpc_status_t err = kErpcStatus_Success;
    while (!err && m_isServerOn)
    {
        err = runInternal();
    }
    return err;
}

#if ERPC_NESTED_CALLS
erpc_status_t SimpleServer::run(RequestContext &request)
{
    erpc_status_t err = kErpcStatus_Success;
    message_type_t msgType;
    uint32_t serviceId;
    uint32_t methodId;
    uint32_t sequence;

    while (!err && m_isServerOn)
    {
        MessageBuffer buff;
        Codec *codec = NULL;

        // Handle the request.
        err = runInternalBegin(&codec, buff, msgType, serviceId, methodId, sequence);

        if (err != kErpcStatus_Success)
        {
            break;
        }

        if (msgType == kReplyMessage)
        {
            if (sequence == request.getSequence())
            {
                // Swap the received message buffer with the client's message buffer.
                request.getCodec()->getBuffer()->swap(&buff);
                codec->setBuffer(buff);
            }

            // Dispose of buffers and codecs.
            disposeBufferAndCodec(codec);

            if (sequence != request.getSequence())
            {
                // Ignore message
                continue;
            }

            break;
        }
        else
        {
            err = runInternalEnd(codec, msgType, serviceId, methodId, sequence);
        }
    }
    return err;
}
#endif

erpc_status_t SimpleServer::poll(void)
{
    erpc_status_t err;

    if (m_isServerOn)
    {
        if (m_transport->hasMessage() == true)
        {
            err = runInternal();
        }
        else
        {
            err = kErpcStatus_Success;
        }
    }
    else
    {
        err = kErpcStatus_ServerIsDown;
    }

    return err;
}

void SimpleServer::stop(void)
{
    m_isServerOn = false;
}
