/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2017-2022 NXP
 * Copyright 2019-2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_rpmsg_tty_rtos_transport.hpp"

#include "erpc_config_internal.h"

extern "C" {
#include "rpmsg_ns.h"
}

using namespace erpc;
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////
uint8_t RPMsgBase::s_initialized = 0U;
struct rpmsg_lite_instance *RPMsgBase::s_rpmsg;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

RPMsgTTYRTOSTransport::RPMsgTTYRTOSTransport(void) :
FramedTransport(), RPMsgBase(), m_dst_addr(0), m_rpmsg_queue(NULL), m_rpmsg_ept(NULL)
{
}

RPMsgTTYRTOSTransport::~RPMsgTTYRTOSTransport(void)
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

        if (!skip && (m_rpmsg_queue != RL_NULL))
        {
            if (RL_SUCCESS != rpmsg_queue_destroy(s_rpmsg, m_rpmsg_queue))
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

erpc_status_t RPMsgTTYRTOSTransport::init(uint32_t src_addr, uint32_t dst_addr, void *base_address, uint32_t length,
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
            m_rpmsg_queue = rpmsg_queue_create(s_rpmsg, m_queue_stack, &m_queue_context);
#else
            m_rpmsg_queue = rpmsg_queue_create(s_rpmsg);
#endif
            if (m_rpmsg_queue == RL_NULL)
            {
                status = kErpcStatus_InitFailed;
            }
        }

        if (status == kErpcStatus_Success)
        {
#if RL_USE_STATIC_API
            m_rpmsg_ept = rpmsg_lite_create_ept(s_rpmsg, src_addr, rpmsg_queue_rx_cb, m_rpmsg_queue, &m_ept_context);
#else
            m_rpmsg_ept = rpmsg_lite_create_ept(s_rpmsg, src_addr, rpmsg_queue_rx_cb, m_rpmsg_queue);
#endif
            if (m_rpmsg_ept == RL_NULL)
            {
                status = kErpcStatus_InitFailed;
            }
            else
            {
                m_dst_addr = dst_addr;
                s_initialized = 1U;
            }
        }

        if (status != kErpcStatus_Success)
        {
            if (m_rpmsg_queue != RL_NULL)
            {
                if (RL_SUCCESS == rpmsg_queue_destroy(s_rpmsg, m_rpmsg_queue))
                {
                    m_rpmsg_queue = NULL;
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

erpc_status_t RPMsgTTYRTOSTransport::init(uint32_t src_addr, uint32_t dst_addr, void *base_address,
                                          uint32_t rpmsg_link_id, void (*ready_cb)(void), char *nameservice_name)
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

            static_cast<void>(rpmsg_lite_wait_for_link_up(s_rpmsg, RL_BLOCK));

#if RL_USE_STATIC_API
            m_rpmsg_queue = rpmsg_queue_create(s_rpmsg, m_queue_stack, &m_queue_context);
#else
            m_rpmsg_queue = rpmsg_queue_create(s_rpmsg);
#endif
            if (m_rpmsg_queue == RL_NULL)
            {
                status = kErpcStatus_InitFailed;
            }
        }

        if (status == kErpcStatus_Success)
        {
#if RL_USE_STATIC_API
            m_rpmsg_ept = rpmsg_lite_create_ept(s_rpmsg, src_addr, rpmsg_queue_rx_cb, m_rpmsg_queue, &m_ept_context);
#else
            m_rpmsg_ept = rpmsg_lite_create_ept(s_rpmsg, src_addr, rpmsg_queue_rx_cb, m_rpmsg_queue);
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
                if (RL_SUCCESS == rpmsg_lite_destroy_ept(s_rpmsg, m_rpmsg_ept))
                {
                    m_rpmsg_ept = NULL;
                }
            }

            if (m_rpmsg_queue != RL_NULL)
            {
                if (RL_SUCCESS == rpmsg_queue_destroy(s_rpmsg, m_rpmsg_queue))
                {
                    m_rpmsg_queue = NULL;
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

erpc_status_t RPMsgTTYRTOSTransport::underlyingSend(MessageBuffer *message, uint32_t size, uint32_t offset)
{
    erpc_status_t status;
    int32_t ret_val;

    ret_val = rpmsg_lite_send_nocopy(s_rpmsg, m_rpmsg_ept, m_dst_addr, &message->get()[offset], size);
    if (ret_val == RL_SUCCESS)
    {

        message->set(NULL, 0);
        status = kErpcStatus_Success;
    }
    else
    {
        status = kErpcStatus_SendFailed;
    }

    return status;
}

erpc_status_t RPMsgTTYRTOSTransport::underlyingReceive(MessageBuffer *message, uint32_t size, uint32_t offset)
{
    char *buf = NULL;
    uint32_t length = 0;
    int32_t ret_val = rpmsg_queue_recv_nocopy(s_rpmsg, m_rpmsg_queue, &m_dst_addr, &buf, &length, RL_BLOCK);
    erpc_status_t status;
    if (ret_val == RL_SUCCESS)
    {
        if (offset == 0)
        {
            message->set(reinterpret_cast<uint8_t *>(buf), length);
            message->setUsed(length);
        }
        else
        {
            erpc_assert(buf != NULL);
            erpc_assert((length + offset) <= ERPC_DEFAULT_BUFFER_SIZE);
            static_cast<void>(memcpy(&message->get()[offset], buf, length));
            message->setUsed(length + offset);
            static_cast<void>(rpmsg_lite_release_rx_buffer(s_rpmsg, buf));
        }
        status = kErpcStatus_Success;
    }
    else
    {
        status = kErpcStatus_ReceiveFailed;
    }

    return status;
}

erpc_status_t RPMsgTTYRTOSTransport::underlyingReceive(uint8_t *data, uint32_t size)
{
    // unused as underlyingReceive(MessageBuffer *message, uint32_t size, uint32_t offset)
    return kErpcStatus_ReceiveFailed;
}

erpc_status_t RPMsgTTYRTOSTransport::underlyingSend(const uint8_t *data, uint32_t size)
{
    // unused as underlyingReceive(MessageBuffer *message, uint32_t size, uint32_t offset)
    return kErpcStatus_SendFailed;
}
