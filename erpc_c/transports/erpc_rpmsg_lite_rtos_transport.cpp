/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_rpmsg_lite_rtos_transport.h"
#include "erpc_config_internal.h"
#include "rpmsg_ns.h"
#include <cassert>

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////
uint8_t RPMsgBaseTransport::s_initialized = 0U;
struct rpmsg_lite_instance *RPMsgBaseTransport::s_rpmsg;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

RPMsgRTOSTransport::RPMsgRTOSTransport(void)
: RPMsgBaseTransport()
, m_rdev(NULL)
, m_app_rp_chnl(NULL)
, m_dst_addr(0)
, m_rpmsg_queue(NULL)
, m_rpmsg_ept(NULL)
{
}

RPMsgRTOSTransport::~RPMsgRTOSTransport(void)
{
    rpmsg_lite_deinit(s_rpmsg);
    s_initialized = 0U;
}

erpc_status_t RPMsgRTOSTransport::init(uint32_t src_addr, uint32_t dst_addr, void *base_address, uint32_t length,
                                       uint32_t rpmsg_link_id)
{
    if (0U == s_initialized)
    {
        s_rpmsg = rpmsg_lite_master_init(base_address, length, rpmsg_link_id, RL_NO_FLAGS);
        if (!s_rpmsg)
        {
            return (erpc_status_t)kErpcStatus_InitFailed;
        }
        s_initialized = 1U;
    }

    m_rpmsg_queue = rpmsg_queue_create(s_rpmsg);
    if (!m_rpmsg_queue)
    {
        return (erpc_status_t)kErpcStatus_InitFailed;
    }

    m_rpmsg_ept = rpmsg_lite_create_ept(s_rpmsg, src_addr, rpmsg_queue_rx_cb, m_rpmsg_queue);

    m_dst_addr = dst_addr;

    return m_rpmsg_ept == RL_NULL ? (erpc_status_t)kErpcStatus_InitFailed : (erpc_status_t)kErpcStatus_Success;
}

erpc_status_t RPMsgRTOSTransport::init(uint32_t src_addr, uint32_t dst_addr, void *base_address, uint32_t rpmsg_link_id,
                                       void (*ready_cb)(void), char *nameservice_name)
{
    if (0U == s_initialized)
    {
        s_rpmsg = rpmsg_lite_remote_init(base_address, rpmsg_link_id, RL_NO_FLAGS);
        if (!s_rpmsg)
        {
            return (erpc_status_t)kErpcStatus_InitFailed;
        }

        /* Signal the other core we are ready */
        if (ready_cb != NULL)
        {
            ready_cb();
        }

        while (0 == rpmsg_lite_is_link_up(s_rpmsg))
        {
        }

        s_initialized = 1U;
    }

    m_rpmsg_queue = rpmsg_queue_create(s_rpmsg);
    if (!m_rpmsg_queue)
    {
        return kErpcStatus_InitFailed;
    }
    m_rpmsg_ept = rpmsg_lite_create_ept(s_rpmsg, src_addr, rpmsg_queue_rx_cb, m_rpmsg_queue);

    if (NULL != nameservice_name)
    {
        if (RL_SUCCESS != rpmsg_ns_announce(s_rpmsg, m_rpmsg_ept, nameservice_name, (uint32_t)RL_NS_CREATE))
        {
            return (erpc_status_t)kErpcStatus_InitFailed;
        }
    }

    m_dst_addr = dst_addr;

    return m_rpmsg_ept == RL_NULL ? (erpc_status_t)kErpcStatus_InitFailed : (erpc_status_t)kErpcStatus_Success;
}

erpc_status_t RPMsgRTOSTransport::receive(MessageBuffer *message)
{
    char *buf = NULL;
    uint32_t length = 0;
    int32_t ret_val = rpmsg_queue_recv_nocopy(s_rpmsg, m_rpmsg_queue, &m_dst_addr, &buf, &length, RL_BLOCK);
    assert(buf);
    message->set((uint8_t *)buf, length);
    message->setUsed(length);
    return ret_val != RL_SUCCESS ? (erpc_status_t)kErpcStatus_ReceiveFailed : (erpc_status_t)kErpcStatus_Success;
}

erpc_status_t RPMsgRTOSTransport::send(MessageBuffer *message)
{
    uint8_t *buf = message->get();
    uint32_t length = message->getLength();
    uint32_t used = message->getUsed();
    message->set(NULL, 0);
    int32_t ret_val = rpmsg_lite_send_nocopy(s_rpmsg, m_rpmsg_ept, m_dst_addr, buf, used);
    if (ret_val == RL_SUCCESS)
    {
        return (erpc_status_t)kErpcStatus_Success;
    }
    message->set(buf, length);
    message->setUsed(used);
    return (erpc_status_t)kErpcStatus_SendFailed;
}
