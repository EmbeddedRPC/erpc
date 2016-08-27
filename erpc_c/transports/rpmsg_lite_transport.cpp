/*
 * Copyright (c) 2015-2016, Freescale Semiconductor, Inc.
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
 * LOSS OF USE, DATA, OR PROFI65TS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "rpmsg_lite_transport.h"
#include <new>

#if !(__embedded_cplusplus)
using namespace std;
#endif

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////
uint8_t RPMsgTransport::s_initialized = 0;
struct rpmsg_lite_instance RPMsgTransport::s_rpmsg_ctxt;
struct rpmsg_lite_instance *RPMsgTransport::s_rpmsg = NULL;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

int RPMsgTransport::rpmsg_read_cb(void *payload, int payload_len, unsigned long src, void *priv)
{
    RPMsgTransport *transport = (RPMsgTransport *)priv;
    if (payload_len <= RPMSG_BUFFER_SIZE)
    {
        MessageBuffer *message = transport->m_freeMessageQueue.get();
        if (message)
        {
            memcpy(message->get(), payload, payload_len);
            message->setUsed(payload_len);
            transport->m_messageQueue.add(*message);
        }
        else
        {
            // TODO: send response - data was not processed, server is busy
        }
    }
    return RL_RELEASE;
}

RPMsgTransport::RPMsgTransport()
: Transport()
, m_dst_addr(0)
{
}

RPMsgTransport::~RPMsgTransport()
{
}

status_t RPMsgTransport::init(
    unsigned long src_addr, unsigned long dst_addr, void *base_address, unsigned long length, int rpmsg_link_id)
{
    int i;
    for (i = 0; i < kInitCountMessageBuffers; i++)
    {
        MessageBuffer message(m_buffers[i], kRpmsgMessageBufferSize);
        m_freeMessageQueue.add(message);
    }

    if (!s_initialized)
    {
        s_rpmsg = rpmsg_lite_master_init(base_address, length, rpmsg_link_id, RL_NO_FLAGS, &s_rpmsg_ctxt);
        s_initialized = 1;
    }

    m_rpmsg_ept = rpmsg_lite_create_ept(s_rpmsg, src_addr, rpmsg_read_cb, this, &m_rpmsg_ept_context);

    m_dst_addr = dst_addr;

    return kErpcStatus_Success;
}

status_t RPMsgTransport::init(
    unsigned long src_addr, unsigned long dst_addr, void *base_address, int rpmsg_link_id, void (*ready_cb)(void))
{
    int i;
    for (i = 0; i < kInitCountMessageBuffers; i++)
    {
        MessageBuffer message(m_buffers[i], kRpmsgMessageBufferSize);
        m_freeMessageQueue.add(message);
    }

    if (!s_initialized)
    {
        s_rpmsg = rpmsg_lite_remote_init(base_address, rpmsg_link_id, RL_NO_FLAGS, &s_rpmsg_ctxt);

        /* Signal the other core we are ready */
        if (ready_cb != NULL)
        {
            ready_cb();
        }

        while (!rpmsg_lite_is_link_up(s_rpmsg))
        {
        }

        s_initialized = 1;
    }

    m_rpmsg_ept = rpmsg_lite_create_ept(s_rpmsg, src_addr, rpmsg_read_cb, this, &m_rpmsg_ept_context);

    m_dst_addr = dst_addr;

    return kErpcStatus_Success;
}

status_t RPMsgTransport::receive(MessageBuffer *message)
{
    while (m_messageQueue.size() < 1)
    {
    }

    uint32_t length = message->getLength();
    uint8_t *freeBuffer = message->get();

    // Change buffers from message and message from messageQueue (filled in rpmsg callback)
    MessageBuffer *messageToRead = m_messageQueue.get();

    uint32_t usedSize = messageToRead->getUsed();
    message->set(messageToRead->get(), messageToRead->getLength());
    message->setUsed(usedSize);

    messageToRead->set(freeBuffer, length);

    // Now the messageToRead contains free buffer
    m_freeMessageQueue.add(*messageToRead);

    return kErpcStatus_Success;
}

status_t RPMsgTransport::send(const MessageBuffer *message)
{
    int ret_value;
    ret_value =
        rpmsg_lite_send(s_rpmsg, m_rpmsg_ept, m_dst_addr, (char *)message->get(), message->getUsed(), RL_DONT_BLOCK);

    if (RL_SUCCESS == ret_value)
    {
        return kErpcStatus_Success;
    }

    return kErpcStatus_Fail;
}

MessageBuffer RPMsgMessageBufferFactory::create()
{
    MessageBuffer *message = m_transport->m_freeMessageQueue.get();
    if (message)
    {
        message->setUsed(0);
        return *message;
    }
    return MessageBuffer();
}

void RPMsgMessageBufferFactory::dispose(MessageBuffer *buf)
{
    m_transport->m_freeMessageQueue.add(*buf);
}
