/*
 * Copyright (c) 2015-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "erpc_rpmsg_lite_transport.h"
#include "erpc_config_internal.h"
#include "rpmsg_ns.h"

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

int32_t RPMsgTransport::rpmsg_read_cb(void *payload, uint32_t payload_len, uint32_t src, void *priv)
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

RPMsgTransport::RPMsgTransport(void)
: RPMsgBaseTransport()
, m_dst_addr(0)
, m_rpmsg_ept_context()
, m_rpmsg_ept(NULL)
{
}

RPMsgTransport::~RPMsgTransport(void) {}

erpc_status_t RPMsgTransport::init(uint32_t src_addr, uint32_t dst_addr, void *base_address, uint32_t length,
                                   int32_t rpmsg_link_id)
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

erpc_status_t RPMsgTransport::init(uint32_t src_addr, uint32_t dst_addr, void *base_address, int32_t rpmsg_link_id,
                                   void (*ready_cb)(void), char *nameservice_name)
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
        if (RL_SUCCESS != rpmsg_ns_announce(s_rpmsg, m_rpmsg_ept, nameservice_name, RL_NS_CREATE))
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
    int32_t ret_val =
        rpmsg_lite_send_nocopy(s_rpmsg, m_rpmsg_ept, m_dst_addr, (char *)message->get(), message->getUsed());
    message->set(NULL, 0);
    return ret_val != RL_SUCCESS ? kErpcStatus_SendFailed : kErpcStatus_Success;
}
