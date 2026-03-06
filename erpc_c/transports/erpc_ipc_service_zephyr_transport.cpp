/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_ipc_service_zephyr_transport.hpp"
#include "erpc_config_internal.h"
#include <new>
#include <cstring>

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

ZephyrIPCTransport::ZephyrIPCTransport(void) :
Transport(), m_ipc_instance(NULL), m_ept(), m_bound_sem(), m_rx_sem(), m_messageQueue(), m_crcImpl(NULL),
m_is_initialized(false)
{
    k_sem_init(&m_bound_sem, 0, 1);
    k_sem_init(&m_rx_sem, 0, K_SEM_MAX_LIMIT);
}

ZephyrIPCTransport::~ZephyrIPCTransport(void)
{
    if (m_is_initialized)
    {
        // Deregister endpoint
        (void)ipc_service_deregister_endpoint(&m_ept);
        m_is_initialized = false;
    }
}

void ZephyrIPCTransport::ept_bound_cb(void *priv)
{
    ZephyrIPCTransport *transport = reinterpret_cast<ZephyrIPCTransport *>(priv);
    if (transport != NULL)
    {
        k_sem_give(&transport->m_bound_sem);
    }
}

void ZephyrIPCTransport::ept_recv_cb(const void *data, size_t len, void *priv)
{
    ZephyrIPCTransport *transport = reinterpret_cast<ZephyrIPCTransport *>(priv);

    if ((transport != NULL) && (data != NULL) && (len > 0U))
    {
        if (len <= ERPC_DEFAULT_BUFFER_SIZE)
        {
            // HOLD the buffer
            int ret = ipc_service_hold_rx_buffer(&transport->m_ept, const_cast<void *>(data));

            if (ret == 0)
            {
                MessageBuffer message(const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(data)),
                                      static_cast<uint16_t>(len));
                message.setUsed(static_cast<uint16_t>(len));

                // Add to queue
                (void)transport->m_messageQueue.add(message);

                // Signal that message is available
                k_sem_give(&transport->m_rx_sem);
            }
            // Note: If hold fails or queue add fails, buffer is NOT held,
            // so IPC backend will free it automatically
        }
    }
}

void ZephyrIPCTransport::ept_error_cb(const char *message, void *priv)
{
    // Handle error - could log or set error state
    (void)priv;
    (void)message;
}

erpc_status_t ZephyrIPCTransport::init(const struct device *ipc_instance, const char *ept_name)
{
    int ret;

    if (m_is_initialized)
    {
        return kErpcStatus_InitFailed;
    }

    if ((ipc_instance == NULL) || (ept_name == NULL))
    {
        return kErpcStatus_InitFailed;
    }

    m_ipc_instance = ipc_instance;

    // Open IPC instance
    ret = ipc_service_open_instance(m_ipc_instance);
    if ((ret < 0) && (ret != -EALREADY))
    {
        return kErpcStatus_InitFailed;
    }

    // Prepare endpoint configuration
    m_ept_cfg.name = ept_name;
    m_ept_cfg.prio = 0;
    m_ept_cfg.cb.bound = ept_bound_cb;
    m_ept_cfg.cb.received = ept_recv_cb;
    m_ept_cfg.cb.error = ept_error_cb;
    m_ept_cfg.cb.unbound = NULL;
    m_ept_cfg.priv = this;

    // Register endpoint
    ret = ipc_service_register_endpoint(m_ipc_instance, &m_ept, &m_ept_cfg);
    if (ret < 0)
    {
        return kErpcStatus_InitFailed;
    }

    // Wait for endpoint to be bound
    ret = k_sem_take(&m_bound_sem, K_FOREVER);
    if (ret != 0)
    {
        (void)ipc_service_deregister_endpoint(&m_ept);
        return kErpcStatus_InitFailed;
    }

    m_is_initialized = true;
    return kErpcStatus_Success;
}

erpc_status_t ZephyrIPCTransport::receive(MessageBuffer *message)
{
    if (!m_is_initialized || (message == NULL))
    {
        return kErpcStatus_ReceiveFailed;
    }

    // Wait for a message to be available
    (void)k_sem_take(&m_rx_sem, K_FOREVER);

    // Get message from queue
    if (m_messageQueue.get(message))
    {
        return kErpcStatus_Success;
    }

    return kErpcStatus_ReceiveFailed;
}

erpc_status_t ZephyrIPCTransport::send(MessageBuffer *message)
{
    int ret;

    if (!m_is_initialized || (message == NULL))
    {
        return kErpcStatus_SendFailed;
    }

    uint8_t *buf = message->get();
    uint16_t len = message->getUsed();

    // Use nocopy send (buffer was allocated by factory with get_tx_buffer)
    ret = ipc_service_send_nocopy(&m_ept, buf, len);

    if (ret < 0)
    {
        return kErpcStatus_SendFailed;
    }

    // Clear the message buffer - ownership transferred to IPC backend
    message->set(NULL, 0);

    return kErpcStatus_Success;
}

bool ZephyrIPCTransport::hasMessage(void)
{
    return (m_messageQueue.size() > 0U);
}

void ZephyrIPCTransport::setCrc16(Crc16 *crcImpl)
{
    erpc_assert(crcImpl != NULL);
    m_crcImpl = crcImpl;
}

Crc16 *ZephyrIPCTransport::getCrc16(void)
{
    return m_crcImpl;
}
