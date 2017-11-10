/*
 * Copyright (c) 2015-2016, Freescale Semiconductor, Inc.
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
#include "rpmsg_lite_transport.h"
#include "rpmsg_ns.h"
#include "erpc_config_internal.h"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////
uint8_t RPMsgBaseTransport::s_initialized = 0;
struct rpmsg_lite_instance RPMsgTransport::s_rpmsg_ctxt;
struct rpmsg_lite_instance *RPMsgBaseTransport::s_rpmsg = NULL;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

int RPMsgTransport::rpmsg_read_cb(void *payload, int payload_len, unsigned long src, void *priv)
{
    RPMsgTransport *transport = (RPMsgTransport *)priv;
    if (payload_len <= ERPC_DEFAULT_BUFFER_SIZE)
    {
        MessageBuffer message((uint8_t *)payload, payload_len);
        message.setUsed(payload_len);
        transport->m_messageQueue.add(message);
    }
    return RL_HOLD;
}

RPMsgTransport::RPMsgTransport()
: RPMsgBaseTransport()
, m_dst_addr(0)
, m_rpmsg_ept_context()
, m_rpmsg_ept(NULL)
{
}

RPMsgTransport::~RPMsgTransport()
{
}

erpc_status_t RPMsgTransport::init(
    unsigned long src_addr, unsigned long dst_addr, void *base_address, unsigned long length, int rpmsg_link_id)
{
    if (!s_initialized)
    {
        s_rpmsg = rpmsg_lite_master_init(base_address, length, rpmsg_link_id, RL_NO_FLAGS, &s_rpmsg_ctxt);
        s_initialized = 1;
    }

    m_rpmsg_ept = rpmsg_lite_create_ept(s_rpmsg, src_addr, rpmsg_read_cb, this, &m_rpmsg_ept_context);

    m_dst_addr = dst_addr;

    return m_rpmsg_ept == RL_NULL ? kErpcStatus_InitFailed : kErpcStatus_Success;
}

erpc_status_t RPMsgTransport::init(
    unsigned long src_addr, unsigned long dst_addr, void *base_address, int rpmsg_link_id, void (*ready_cb)(void), char *nameservice_name)
{
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

    if (nameservice_name)
    {
        if (RL_SUCCESS != rpmsg_ns_announce(s_rpmsg, m_rpmsg_ept, nameservice_name,
                                            RL_NS_CREATE))
        {
            return kErpcStatus_InitFailed;
        }
    }

    m_dst_addr = dst_addr;

    return m_rpmsg_ept == RL_NULL ? kErpcStatus_InitFailed : kErpcStatus_Success;
}

erpc_status_t RPMsgTransport::receive(MessageBuffer *message)
{
    while (!m_messageQueue.get(message))
    {
    }

    return kErpcStatus_Success;
}

erpc_status_t RPMsgTransport::send(MessageBuffer *message)
{
    int ret_val =
        rpmsg_lite_send_nocopy(s_rpmsg, m_rpmsg_ept, m_dst_addr, (char *)message->get(), message->getUsed());
    message->set(NULL, 0);
    return ret_val != RL_SUCCESS ? kErpcStatus_SendFailed : kErpcStatus_Success;
}
