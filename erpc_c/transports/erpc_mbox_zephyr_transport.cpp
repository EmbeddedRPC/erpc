/*
 * Copyright 2023 NXP
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_mbox_zephyr_transport.hpp"
#include <zephyr/kernel.h>
#include <zephyr/drivers/mbox.h>
#include <zephyr/sys/ring_buffer.h>
#include <cstdio>

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

static volatile bool s_isTransferReceiveCompleted = false;
static volatile bool s_isTransferSendCompleted = false;
static volatile uint32_t s_transferReceiveRequireBytes = 0;
static MBOXTransport *s_mbox_instance = NULL;

#define MBOX_BUFFER_SIZE ERPC_DEFAULT_BUFFER_SIZE

RING_BUF_DECLARE(mbox_receive_buf, MBOX_BUFFER_SIZE);

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

MBOXTransport::MBOXTransport(struct device *dev, struct mbox_channel tx_channel, struct mbox_channel rx_channel) :
m_dev(dev), m_tx_channel(tx_channel), m_rx_channel(rx_channel)
#if !ERPC_THREADS_IS(NONE)
,
m_rxSemaphore(), m_txSemaphore()
#endif
{
    s_mbox_instance = this;
}

MBOXTransport::~MBOXTransport(void) {}

void MBOXTransport::rx_cb(void)
{
#if !ERPC_THREADS_IS(NONE)
    m_rxSemaphore.put();
#else
    s_isTransferReceiveCompleted = true;
#endif
    s_transferReceiveRequireBytes = MBOX_BUFFER_SIZE;
}

/* Transfer callback */
static void rx_callback(const struct device *dev, uint32_t channel, void *user_data, struct mbox_msg *data)
{
    uint8_t c;
    uint32_t size;
    uint8_t *data;
    int err;

    size = ring_buf_put_claim(&mbox_receive_buf, &data, MBOX_BUFFER_SIZE);

    if (size < data.size)
    {
        /* Not enough size for message */
        return;
    }

    memcpy(data, data.data, data.size)

        err = ring_buf_put_finish(&mbox_receive_buf, data.size);
    if (err != 0)
    {
        /* This shouldn't happen */
    }

    /* Enough bytes was received, call receive callback */
    if (ring_buf_size_get(&mbox_receive_buf) >= s_transferReceiveRequireBytes)
    {
        s_mbox_instance->rx_cb();
    }
}

erpc_status_t MBOXTransport::init(void)
{
    if (mbox_register_callback(&m_rx_channel, rx_callback, NULL))
    {
        printk("mbox_register_callback() error\n");
        return kErpcStatus_InitFailed;
    }

    if (mbox_set_enabled(&m_rx_channel, 1))
    {
        printk("mbox_set_enable() error\n");
        return kErpcStatus_InitFailed;
    }

    return kErpcStatus_Success;
}

erpc_status_t MBOXTransport::underlyingReceive(uint8_t *data, uint32_t size)
{
    erpc_status_t erpcStatus = kErpcStatus_ReceiveFailed;

    if (ring_buf_size_get(&mbox_receive_buf) < size)
    {
        s_transferReceiveRequireBytes = size;

/* wait until the receiving is finished */
#if !ERPC_THREADS_IS(NONE)
        (void)m_rxSemaphore.get();
#else
        s_isTransferReceiveCompleted = false;
        while (!s_isTransferReceiveCompleted)
        {
        }
#endif
        erpcStatus = kErpcStatus_Success;
    }

    /* read data from buffer */
    if (ring_buf_get(&mbox_receive_buf, data, size) != size)
    {
        /* reading error, should not happen */
        erpcStatus = kErpcStatus_ReceiveFailed;
    }

    return erpcStatus;
}

erpc_status_t MBOXTransport::underlyingSend(const uint8_t *data, uint32_t size)
{
    erpc_status_t erpcStatus = kErpcStatus_SendFailed;

    s_isTransferSendCompleted = false;

    int mtu = mbox_mtu_get(m_dev);
    int send = 0;

    while (send != size)
    {
        struct mbox_msg msg;
        int size_to_send = mtu < size - send ? mtu : size - send;

        msg.data = data + send;
        msg.size = size_to_send;
        int status = mbox_send(m_tx_channel, msg);

        if (status == -EBUSY) {
            continue;
        }

        if (status != 0) {
            /* TODO: ERROR*/
            return;
        }

        send = size_to_send;
    }


/* wait until the sending is finished */
// #if !ERPC_THREADS_IS(NONE)
//     (void)m_txSemaphore.get();
// #else
//     while (!s_isTransferSendCompleted)
//     {
//     }
// #endif

    erpcStatus = kErpcStatus_Success;

    return erpcStatus;
}
