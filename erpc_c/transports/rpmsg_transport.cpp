/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
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

#include "rpmsg_transport.h"
#include <new>

#if !(__embedded_cplusplus)
using namespace std;
#endif

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

/* Queue of messages with buffers filled in rpmsg callback */
StaticQueue<MessageBuffer *> RPMsgTransport::messageQueue(kInitCountMessageBuffers + 1);

// Queue of messages with free buffers
StaticQueue<MessageBuffer *> RPMsgTransport::freeMessageQueue(kInitCountMessageBuffers + 1);

/* Remote device */
struct remote_device *RPMsgTransport::m_rdev = NULL;
struct rpmsg_endpoint *RPMsgTransport::m_rp_ept = NULL;
struct rpmsg_channel *volatile RPMsgTransport::m_app_rp_chnl = NULL;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

/* This callback gets invoked when the remote channel is created */
void RPMsgTransport::rpmsg_channel_created(struct rpmsg_channel *rp_chnl)
{
    m_app_rp_chnl = rp_chnl;
    m_rp_ept = rpmsg_create_ept(rp_chnl, rpmsg_read_cb, RPMSG_NULL, RPMSG_ADDR_ANY);
}

/* This callback gets invoked when the remote channel is deleted */
void RPMsgTransport::rpmsg_channel_deleted(struct rpmsg_channel *rp_chnl)
{
    rpmsg_destroy_ept(m_rp_ept);
}

/* This is the read callback, note we are in a task context when this callback
   is invoked, so kernel primitives can be used freely */
void RPMsgTransport::rpmsg_read_cb(struct rpmsg_channel *rp_chnl, void *data, int len, void *priv, unsigned long src)
{
    MessageBuffer *message = freeMessageQueue.get();
    if (message)
    {
        memcpy(message->get(), data, len);
        message->setUsed(len);
        messageQueue.add(message);
    }
    else
    {
        // TODO: send response - data was not processed, server is busy
    }
}

RPMsgTransport::RPMsgTransport()
: Transport()
{
}

RPMsgTransport::~RPMsgTransport()
{
    rpmsg_deinit(m_rdev);
}

void RPMsgTransport::clearMessageQueue(StaticQueue<MessageBuffer *> *messageQueue)
{
    MessageBuffer *tmp_buff = messageQueue->get();
    while (tmp_buff)
    {
        delete[] tmp_buff->get();
        delete tmp_buff;
        tmp_buff = messageQueue->get();
    }
}

status_t RPMsgTransport::init(int dev_id, int role)
{
    if (!messageQueue.init())
    {
        return kErpcStatus_Fail;
    }

    if (!freeMessageQueue.init())
    {
        return kErpcStatus_Fail;
    }

    // Allocate a few buffers, which will be used in rpmsg rx callback for incoming data
    int i;
    for (i = 0; i < kInitCountMessageBuffers; i++)
    {
        uint8_t *buf = new (nothrow) uint8_t[kRpmsgMessageBufferSize];
        if (buf)
        {
            MessageBuffer *message = new (nothrow) MessageBuffer(buf, kRpmsgMessageBufferSize);
            if (message)
            {
                freeMessageQueue.add(message);
            }
            else
            {
                delete[] buf;
                clearMessageQueue(&freeMessageQueue);
                return kErpcStatus_Fail;
            }
        }
        else
        {
            clearMessageQueue(&freeMessageQueue);
            return kErpcStatus_Fail;
        }
    }

    int ret_value;
    ret_value = rpmsg_init(dev_id, &m_rdev, rpmsg_channel_created, rpmsg_channel_deleted, rpmsg_read_cb, role);
    if (ret_value != RPMSG_SUCCESS)
    {
        return kErpcStatus_Fail;
    }

    return kErpcStatus_Success;
}

status_t RPMsgTransport::receive(MessageBuffer *message)
{
    while (messageQueue.size() < 1)
    {
    }

    uint32_t length = message->getLength();
    uint8_t *freeBuffer = message->get();

    // Change buffers from message and message from messageQueue (filled in rpmsg callback)
    MessageBuffer *messageToRead = messageQueue.get();

    uint32_t usedSize = messageToRead->getUsed();
    message->set(messageToRead->get(), messageToRead->getLength());
    message->setUsed(usedSize);

    messageToRead->set(freeBuffer, length);

    // Now the messageToRead contains free buffer
    freeMessageQueue.add(messageToRead);

    return kErpcStatus_Success;
}

status_t RPMsgTransport::send(const MessageBuffer *message)
{
    while (m_app_rp_chnl == NULL)
    {
    }

    int ret_value;
    ret_value = rpmsg_send((struct rpmsg_channel *)m_app_rp_chnl, (void *)message->get(), message->getUsed());

    if (RPMSG_SUCCESS == ret_value)
    {
        return kErpcStatus_Success;
    }

    return kErpcStatus_Fail;
}

MessageBuffer RPMsgMessageBufferFactory::create()
{
    uint8_t *buf = new (nothrow) uint8_t[kRpmsgMessageBufferSize];
    assert(buf);

    return MessageBuffer(buf, kRpmsgMessageBufferSize);
}

void RPMsgMessageBufferFactory::dispose(MessageBuffer *buf)
{
    assert(buf);
    if (buf->get())
    {
        delete[] buf->get();
    }
}
