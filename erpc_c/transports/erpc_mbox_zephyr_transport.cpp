/*
 * Copyright 2023-2026 NXP
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_mbox_zephyr_transport.hpp"
#include "erpc_port.h"
#include <zephyr/kernel.h>
#include <cstdio>

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////
static MBOXTransport *s_mbox_instance = NULL;

RING_BUF_DECLARE(s_rxRingBuffer, MBOX_BUFFER_SIZE);

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

MBOXTransport::MBOXTransport(struct device *dev, struct mbox_dt_spec *tx_channel, struct mbox_dt_spec *rx_channel) :
m_dev(dev), m_tx_channel(tx_channel), m_rx_channel(rx_channel)
#if !ERPC_THREADS_IS(NONE)
,
m_rxSemaphore(), m_txSemaphore()
#endif
{
    s_mbox_instance = this;
}

MBOXTransport::~MBOXTransport(void) {}

void MBOXTransport::rx_cb(struct mbox_msg *data)
{
    // Read data to ring buffer
    ring_buf_put(&s_rxRingBuffer, static_cast<const uint8_t *>(data->data), data->size);

    // message is complete, unblock caller of the receive function
    if (ring_buf_size_get(&s_rxRingBuffer) >= m_transferReceiveRequireBytes)
    {
        m_isTransferReceiveCompleted = true;
#if !ERPC_THREADS_IS(NONE)
        // disable MU rx full interrupt in rtos-based blocking implementation
        m_rxSemaphore.put();
#endif
        m_transferReceiveRequireBytes = MBOX_BUFFER_SIZE;
    }
}

/* Transfer callback */
static void mbox_callback(const struct device *dev, uint32_t channel, void *user_data, struct mbox_msg *data)
{
    s_mbox_instance->rx_cb(data);
}

erpc_status_t MBOXTransport::init(void)
{
    if (mbox_register_callback_dt(m_rx_channel, mbox_callback, NULL))
    {
        printk("mbox_register_callback() error\n");
        return kErpcStatus_InitFailed;
    }

    if (mbox_set_enabled_dt(m_rx_channel, true))
    {
        printk("mbox_set_enable() error\n");
        return kErpcStatus_InitFailed;
    }

    return kErpcStatus_Success;
}

erpc_status_t MBOXTransport::receive(MessageBuffer *message)
{
    erpc_status_t status = kErpcStatus_Success;
    uint8_t tmp[4];
    uint32_t rxMsgSize = 0;

    if (message == NULL)
    {
        status = kErpcStatus_ReceiveFailed;
    }
    else
    {
#if !ERPC_THREADS_IS(NONE)
        Mutex::Guard lock(m_receiveLock);
#endif

        // Wait for size of the message
        waitForBytes(sizeof(uint32_t));
        ring_buf_get(&s_rxRingBuffer, (uint8_t *)&rxMsgSize, sizeof(uint32_t));

        // Wait for message to be transmitted
        waitForBytes(rxMsgSize);
        message->setUsed((uint16_t)rxMsgSize);

        if (ring_buf_get(&s_rxRingBuffer, reinterpret_cast<uint8_t *>(message->get()), rxMsgSize) != rxMsgSize)
        {
            status = kErpcStatus_ReceiveFailed;
        };

        // Read remaining bytes from ring buffer
        ring_buf_get(&s_rxRingBuffer, tmp, 4 - (rxMsgSize % 4));

        rxMsgSize = 0;
    }

    return status;
}

void MBOXTransport::waitForBytes(uint32_t numOfBytes)
{
    if (ring_buf_size_get(&s_rxRingBuffer) < numOfBytes)
    {
        m_transferReceiveRequireBytes = numOfBytes;

/* wait until the receiving is finished */
#if !ERPC_THREADS_IS(NONE)
        (void)m_rxSemaphore.get();
#else
        m_isTransferReceiveCompleted = false;
        while (!m_isTransferReceiveCompleted)
        {
        }
#endif
    }
}

erpc_status_t MBOXTransport::send(MessageBuffer *message)
{
    erpc_status_t status = kErpcStatus_Success;
    struct mbox_msg txMsg;
    uint32_t txMsgSize = 0;
    uint32_t txCntBytes = 0;
    uint8_t *txBuffer;

    if (message == NULL)
    {
        status = kErpcStatus_SendFailed;
    }
    else
    {
#if !ERPC_THREADS_IS(NONE)
        Mutex::Guard lock(m_sendLock);
#endif

        txMsgSize = message->getUsed();
        txCntBytes = 0;
        txBuffer = message->get();

        txMsg.data = &txMsgSize;
        txMsg.size = sizeof(uint32_t);

        mbox_send_dt(m_tx_channel, &txMsg);

        while (txCntBytes < txMsgSize)
        {
            txMsg.data = &txBuffer[txCntBytes];
            txMsg.size = sizeof(uint32_t);

            int mboxStatus = mbox_send_dt(m_tx_channel, &txMsg);

            if (mboxStatus == -EBUSY)
            {
                continue;
            }

            if (mboxStatus < 0)
            {
                return kErpcStatus_SendFailed;
            }

            txCntBytes += sizeof(uint32_t);
        }

        txMsgSize = 0;
        txCntBytes = 0;
        txBuffer = NULL;
    }

    return status;
}

bool MBOXTransport::hasMessage(void)
{
    return !ring_buf_is_empty(&s_rxRingBuffer);
}
