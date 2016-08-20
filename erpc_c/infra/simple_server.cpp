/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
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
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
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

#include "simple_server.h"

using namespace erpc;
#if !(__embedded_cplusplus)
using namespace std;
#endif

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

SimpleServer::~SimpleServer()
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

Codec *SimpleServer::createBufferAndCodec()
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
        disposeBufferAndCodec(codec);
        return NULL;
    }

    codec->setBuffer(message);

    return codec;
}

status_t SimpleServer::runInternal()
{
    // Create codec to read the request.
    Codec *inCodec = createBufferAndCodec();
    if (!inCodec)
    {
        return kErpcStatus_MemoryError;
    }

    // Receive the next invocation request.
    status_t err = m_transport->receive(inCodec->getBuffer());
    if (err)
    {
        // Dispose of buffers and codecs.
        disposeBufferAndCodec(inCodec);
        return err;
    }

    // If is used RPMsg transport layer, the receive function changes
    // pointer to buffer(m_buf) of requestMessage, inCodec must be reseted
    // after this change to work with correct buffer
    inCodec->reset();

    // Create codec to write the request.
    Codec *outCodec = createBufferAndCodec();
    if (!outCodec)
    {
        // Dispose of buffers and codecs.
        disposeBufferAndCodec(inCodec);
        return kErpcStatus_MemoryError;
    }

    // Handle the request.
    message_type_t msgType;
    err = processMessage(inCodec, outCodec, msgType);
    if (err)
    {
        // Dispose of buffers and codecs.
        disposeBufferAndCodec(outCodec);
        disposeBufferAndCodec(inCodec);
        return err;
    }

    if (msgType != kOnewayMessage)
    {
        err = m_transport->send(outCodec->getBuffer());
    }

    // Dispose of buffers and codecs.
    disposeBufferAndCodec(outCodec);
    disposeBufferAndCodec(inCodec);

    return err;
}

status_t SimpleServer::run()
{
    status_t err = kErpcStatus_Success;
    while (!err && m_isServerOn)
    {
        err = runInternal();
    }
    return err;
}

status_t SimpleServer::poll()
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

void SimpleServer::stop()
{
    m_isServerOn = false;
}
