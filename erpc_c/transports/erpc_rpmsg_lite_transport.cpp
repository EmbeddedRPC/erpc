/*
 * Copyright (c) 2015-2021, Freescale Semiconductor, Inc.
 * Copyright 2016-2022 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
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
uint8_t RPMsgBaseTransport::s_initialized = 0U;
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
        message.setUsed((uint16_t)payload_len);
        (void)transport->m_messageQueue.add(message);
    }
    return RL_HOLD;
}

RPMsgTransport::RPMsgTransport(void)
: RPMsgBaseTransport()
, m_dst_addr(0)
, m_rpmsg_ept(NULL)
{
}

RPMsgTransport::~RPMsgTransport(void)
{
    bool skip = false;
    if (s_rpmsg != RL_NULL)
    {
        if (m_rpmsg_ept != RL_NULL)
        {
            if (RL_SUCCESS != rpmsg_lite_destroy_ept(s_rpmsg, m_rpmsg_ept))
            {
                skip = true;
            }
        }

        if (!skip && (RL_SUCCESS != rpmsg_lite_deinit(s_rpmsg)))
        {
            skip = true;
        }

        if (!skip)
        {
            s_initialized = 0U;
        }
    }
}

erpc_status_t RPMsgTransport::init(uint32_t src_addr, uint32_t dst_addr, void *base_address, uint32_t length,
                                   uint32_t rpmsg_link_id)
{
    erpc_status_t status = kErpcStatus_Success;

    if (0U == s_initialized)
    {
#if RL_USE_STATIC_API
        s_rpmsg = rpmsg_lite_master_init(base_address, length, rpmsg_link_id, RL_NO_FLAGS, &m_static_context);
#else
        s_rpmsg = rpmsg_lite_master_init(base_address, length, rpmsg_link_id, RL_NO_FLAGS);
#endif
        if (s_rpmsg == RL_NULL)
        {
            status = kErpcStatus_InitFailed;
        }

        if (status == kErpcStatus_Success)
        {
#if RL_USE_STATIC_API
            m_rpmsg_ept = rpmsg_lite_create_ept(s_rpmsg, src_addr, rpmsg_read_cb, this, &m_ept_context);
#else
            m_rpmsg_ept = rpmsg_lite_create_ept(s_rpmsg, src_addr, rpmsg_read_cb, this);
#endif
            if (m_rpmsg_ept == RL_NULL)
            {
                status = kErpcStatus_InitFailed;
            }
        }

        if (status == kErpcStatus_Success)
        {
            s_initialized = 1U;
            m_dst_addr = dst_addr;
        }
        else
        {
            if (s_rpmsg != RL_NULL)
            {
                if (RL_SUCCESS == rpmsg_lite_deinit(s_rpmsg))
                {
                    s_rpmsg = NULL;
                }
            }
        }
    }

    return status;
}

erpc_status_t RPMsgTransport::init(uint32_t src_addr, uint32_t dst_addr, void *base_address, uint32_t rpmsg_link_id,
                                   void (*ready_cb)(void), char *nameservice_name)
{
    erpc_status_t status = kErpcStatus_Success;

    if (0U == s_initialized)
    {
#if RL_USE_STATIC_API
        s_rpmsg = rpmsg_lite_remote_init(base_address, rpmsg_link_id, RL_NO_FLAGS, &m_static_context);
#else
        s_rpmsg = rpmsg_lite_remote_init(base_address, rpmsg_link_id, RL_NO_FLAGS);
#endif
        if (s_rpmsg == RL_NULL)
        {
            status = kErpcStatus_InitFailed;
        }

        if (status == kErpcStatus_Success)
        {
            /* Signal the other core we are ready */
            if (ready_cb != NULL)
            {
                ready_cb();
            }

            rpmsg_lite_wait_for_link_up(s_rpmsg);

#if RL_USE_STATIC_API
            m_rpmsg_ept = rpmsg_lite_create_ept(s_rpmsg, src_addr, rpmsg_read_cb, this, &m_ept_context);
#else
            m_rpmsg_ept = rpmsg_lite_create_ept(s_rpmsg, src_addr, rpmsg_read_cb, this);
#endif
            if (m_rpmsg_ept == RL_NULL)
            {
                status = kErpcStatus_InitFailed;
            }
        }

        if (status == kErpcStatus_Success)
        {
            if (NULL != nameservice_name)
            {
                if (RL_SUCCESS != rpmsg_ns_announce(s_rpmsg, m_rpmsg_ept, nameservice_name, (uint32_t)RL_NS_CREATE))
                {
                    status = kErpcStatus_InitFailed;
                }
            }
        }

        if (status == kErpcStatus_Success)
        {
            m_dst_addr = dst_addr;
            s_initialized = 1U;
        }
        else
        {
            if (m_rpmsg_ept != RL_NULL)
            {
                if (RL_SUCCESS != rpmsg_lite_destroy_ept(s_rpmsg, m_rpmsg_ept))
                {
                    m_rpmsg_ept = NULL;
                }
            }

            if (s_rpmsg != RL_NULL)
            {
                if (RL_SUCCESS == rpmsg_lite_deinit(s_rpmsg))
                {
                    s_rpmsg = NULL;
                }
            }
        }
    }

    return status;
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

    return (ret_val != RL_SUCCESS) ? kErpcStatus_SendFailed : kErpcStatus_Success;
}
