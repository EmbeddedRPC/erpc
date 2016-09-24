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
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "rpmsg_lite_rtos_transport.h"
#include <cassert>

#if !(__embedded_cplusplus)
using namespace std;
#endif

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////
uint8_t RPMsgRTOSTransport::s_initialized = 0;
struct rpmsg_lite_instance *RPMsgRTOSTransport::s_rpmsg;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

RPMsgRTOSTransport::RPMsgRTOSTransport()
: Transport()
, m_dst_addr(0)
{
}

RPMsgRTOSTransport::~RPMsgRTOSTransport()
{
    rpmsg_lite_deinit(s_rpmsg);
    s_initialized = 0;
}

erpc_status_t RPMsgRTOSTransport::init(
    unsigned long src_addr, unsigned long dst_addr, void *base_address, unsigned long length, int rpmsg_link_id)
{
    if (!s_initialized)
    {
        s_rpmsg = rpmsg_lite_master_init(base_address, length, rpmsg_link_id, RL_NO_FLAGS);
        s_initialized = 1;
    }

    m_rpmsg_queue = rpmsg_queue_create(s_rpmsg);
    m_rpmsg_ept = rpmsg_lite_create_ept(s_rpmsg, src_addr, rpmsg_queue_rx_cb, m_rpmsg_queue);

    m_dst_addr = dst_addr;
    return m_rpmsg_ept == RL_NULL ? kErpcStatus_InitFailed : kErpcStatus_Success;
}

erpc_status_t RPMsgRTOSTransport::init(
    unsigned long src_addr, unsigned long dst_addr, void *base_address, int rpmsg_link_id, void (*ready_cb)(void))
{
    if (!s_initialized)
    {
        s_rpmsg = rpmsg_lite_remote_init(base_address, rpmsg_link_id, RL_NO_FLAGS);

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

    m_rpmsg_queue = rpmsg_queue_create(s_rpmsg);
    m_rpmsg_ept = rpmsg_lite_create_ept(s_rpmsg, src_addr, rpmsg_queue_rx_cb, m_rpmsg_queue);

    m_dst_addr = dst_addr;
    return m_rpmsg_ept == RL_NULL ? kErpcStatus_InitFailed : kErpcStatus_Success;
}

erpc_erpc_status_t RPMsgRTOSTransport::receive(MessageBuffer *message)
{
    int ret_val = rpmsg_queue_recv(s_rpmsg, m_rpmsg_queue, &m_dst_addr, (char *)message->get(), kRpmsgMessageBufferSize,
                                   NULL, RL_BLOCK);
    return ret_val != RL_SUCCESS ? kErpcStatus_ReceiveFailed : kErpcStatus_Success;
}

erpc_erpc_status_t RPMsgRTOSTransport::send(const MessageBuffer *message)
{
    int ret_val =
        rpmsg_lite_send(s_rpmsg, m_rpmsg_ept, m_dst_addr, (char *)message->get(), message->getUsed(), RL_BLOCK);
    return ret_val != RL_SUCCESS ? kErpcStatus_SendFailed : kErpcStatus_Success;
}

MessageBuffer RPMsgMessageBufferFactory::create()
{
    uint8_t idx = 0;
    while (((m_freeBufferBitmap & idx) == 0) && (idx < kInitCountMessageBuffers))
    {
        idx++;
    }

    assert(idx < kInitCountMessageBuffers);

    m_freeBufferBitmap &= ~(1 << idx);

    uint8_t *buf;
    buf = m_buffers[idx];

    assert(NULL != buf);
    return MessageBuffer(buf, kRpmsgMessageBufferSize);
}

void RPMsgMessageBufferFactory::dispose(MessageBuffer *buf)
{
    assert(buf);
    uint8_t *tmp = buf->get();

    if (tmp)
    {
        uint8_t idx = 0;
        while ((tmp != m_buffers[idx]) && (idx < kInitCountMessageBuffers))
        {
            ++idx;
        }
        m_freeBufferBitmap |= 1 << idx;
    }
}
