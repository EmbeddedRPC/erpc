/*
 * The Clear BSD License
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 * that the following conditions are met:
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
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
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

#include "erpc_simple_server.h"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

SimpleServer::~SimpleServer(void)
{
    while (m_firstService != NULL)
    {
        Service *firstService = m_firstService;
        m_firstService = m_firstService->getNext();
        delete firstService;
    }
}

void SimpleServer::disposeBufferAndCodec(Codec *codec)
{
    if (codec)
    {
        if (codec->getBuffer())
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
    uint32_t methodId;
    uint32_t sequence;

    erpc_status_t err = runInternalBegin(&codec, buff, msgType, serviceId, methodId, sequence);
    if (err)
    {
        return err;
    }

    return runInternalEnd(codec, msgType, serviceId, methodId, sequence);
}

erpc_status_t SimpleServer::runInternalBegin(Codec **codec, MessageBuffer &buff, message_type_t &msgType,
                                             uint32_t &serviceId, uint32_t &methodId, uint32_t &sequence)
{
    if (m_messageFactory->createServerBuffer())
    {
        buff = m_messageFactory->create();
        if (!buff.get())
        {
            return kErpcStatus_MemoryError;
        }
    }

    // Receive the next invocation request.
    erpc_status_t err = m_transport->receive(&buff);
    if (err)
    {
        // Dispose of buffers.
        if (buff.get())
        {
            m_messageFactory->dispose(&buff);
        }
        return err;
    }

#if ERPC_MESSAGE_LOGGING
    err = logMessage(&buff);
    if (err)
    {
        // Dispose of buffers.
        if (buff.get())
        {
            m_messageFactory->dispose(&buff);
        }
        return err;
    }
#endif

    *codec = m_codecFactory->create();
    if (!*codec)
    {
        m_messageFactory->dispose(&buff);
        return kErpcStatus_MemoryError;
    }

    (*codec)->setBuffer(buff);

    err = readHeadOfMessage(*codec, msgType, serviceId, methodId, sequence);
    if (err)
    {
        // Dispose of buffers and codecs.
        disposeBufferAndCodec(*codec);
    }
    return err;
}

erpc_status_t SimpleServer::runInternalEnd(Codec *codec, message_type_t msgType, uint32_t serviceId, uint32_t methodId,
                                           uint32_t sequence)
{
    erpc_status_t err = processMessage(codec, msgType, serviceId, methodId, sequence);

    if (err)
    {
        // Dispose of buffers and codecs.
        disposeBufferAndCodec(codec);
        return err;
    }

    if (msgType != kOnewayMessage)
    {

#if ERPC_MESSAGE_LOGGING
        err = logMessage(codec->getBuffer());
        if (err)
        {
            // Dispose of buffers and codecs.
            disposeBufferAndCodec(codec);
            return err;
        }
#endif
        err = m_transport->send(codec->getBuffer());
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
    while (!err && m_isServerOn)
    {
        MessageBuffer buff;
        Codec *codec = NULL;

        // Handle the request.
        message_type_t msgType;
        uint32_t serviceId;
        uint32_t methodId;
        uint32_t sequence;

        erpc_status_t err = runInternalBegin(&codec, buff, msgType, serviceId, methodId, sequence);

        if (err)
        {
            return err;
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

            return kErpcStatus_Success;
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
    if (m_isServerOn)
    {
        if (m_transport->hasMessage())
        {
            return runInternal();
        }
        else
        {
            return kErpcStatus_Success;
        }
    }
    return kErpcStatus_ServerIsDown;
}

void SimpleServer::stop(void)
{
    m_isServerOn = false;
}
