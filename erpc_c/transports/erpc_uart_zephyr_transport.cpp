/*
 * Copyright 2023 NXP
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_uart_zephyr_transport.hpp"
#include <zephyr/kernel.h>
#include <zephyr/sys/ring_buffer.h>
#include <cstdio>

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

static volatile bool s_isTransferReceiveCompleted = false;
static volatile bool s_isTransferSendCompleted = false;
static volatile uint32_t s_transferReceiveRequireBytes = 0;
static UartTransport *s_uart_instance = NULL;

#define UART_BUFFER_SIZE ERPC_DEFAULT_BUFFER_SIZE

RING_BUF_DECLARE(uart_receive_buf, UART_BUFFER_SIZE);
RING_BUF_DECLARE(uart_send_buf, UART_BUFFER_SIZE);

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

UartTransport::UartTransport(struct device *dev) :
m_dev(dev)
#if !ERPC_THREADS_IS(NONE)
,
m_rxSemaphore(), m_txSemaphore()
#endif
{
    s_uart_instance = this;
}

UartTransport::~UartTransport(void) {}

void UartTransport::tx_cb(void)
{
#if !ERPC_THREADS_IS(NONE)
    m_txSemaphore.put();
#else
    s_isTransferSendCompleted = true;
#endif
}

void UartTransport::rx_cb(void)
{
#if !ERPC_THREADS_IS(NONE)
    m_rxSemaphore.put();
#else
    s_isTransferReceiveCompleted = true;
#endif
    s_transferReceiveRequireBytes = UART_BUFFER_SIZE;
}

/* Transfer callback */
static void TransferCallback(const struct device *dev, void *user_data)
{
    uint8_t c;
    uint32_t size;
    uint32_t rx_size;
    uint32_t tx_size;
    uint8_t *data;
    int err;

    if (!uart_irq_update(dev))
    {
        return;
    }

    if (uart_irq_rx_ready(dev))
    {
        size = ring_buf_put_claim(&uart_receive_buf, &data, UART_BUFFER_SIZE);

        /* read until FIFO empty */
        rx_size = uart_fifo_read(dev, data, size);

        if (rx_size < 0)
        {
            /* Error */
        }

        if (rx_size == size)
        {
            if (uart_fifo_read(dev, &c, 1) == 1)
            {
                /* Error - more data in fifo */
            }
        }

        err = ring_buf_put_finish(&uart_receive_buf, rx_size);
        if (err != 0)
        {
            /* This shouldn't happen unless rx_size > size */
        }

        /* Enough bytes was received, call receive callback */
        if (ring_buf_size_get(&uart_receive_buf) >= s_transferReceiveRequireBytes)
        {
            s_uart_instance->rx_cb();
        }
    }

    if (uart_irq_tx_ready(dev) && ring_buf_size_get(&uart_send_buf) > 0)
    {
        /* read one byte from queue */
        size = ring_buf_get_claim(&uart_send_buf, &data, UART_BUFFER_SIZE);

        /* send it thought UART */
        tx_size = uart_fifo_fill(dev, data, size);
        if (tx_size < 0)
        {
            /* Error */
        }

        err = ring_buf_get_finish(&uart_send_buf, tx_size);
        if (err != 0)
        {
            /* bytes_send exceeds amount of valid data in a ring buffer. */
        }

        if (ring_buf_size_get(&uart_send_buf) == 0)
        {
            /* no more bytes to send; disable tx interrupt */
            uart_irq_tx_disable(dev);
            s_uart_instance->tx_cb();
        }
    }
}

erpc_status_t UartTransport::init(void)
{
    erpc_status_t erpcStatus = kErpcStatus_InitFailed;

    int32_t status = uart_irq_callback_user_data_set(m_dev, TransferCallback, this);

    if (status == 0)
    {
        uart_irq_rx_enable(m_dev);
        erpcStatus = kErpcStatus_Success;
    }

    return erpcStatus;
}

erpc_status_t UartTransport::underlyingReceive(uint8_t *data, uint32_t size)
{
    erpc_status_t erpcStatus = kErpcStatus_Success;

    if (ring_buf_size_get(&uart_receive_buf) < size)
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
    }

    /* read data from buffer */
    if (ring_buf_get(&uart_receive_buf, data, size) != size)
    {
        /* reading error, should not happen */
        erpcStatus = kErpcStatus_ReceiveFailed;
    }

    return erpcStatus;
}

erpc_status_t UartTransport::underlyingSend(const uint8_t *data, uint32_t size)
{
    erpc_status_t erpcStatus = kErpcStatus_SendFailed;

    s_isTransferSendCompleted = false;

    /* fill buffer */
    ring_buf_put(&uart_send_buf, data, size);

    /* enable transport */
    uart_irq_tx_enable(m_dev);

/* wait until the sending is finished */
#if !ERPC_THREADS_IS(NONE)
    (void)m_txSemaphore.get();
#else
    while (!s_isTransferSendCompleted)
    {
    }
#endif

    erpcStatus = kErpcStatus_Success;

    return erpcStatus;
}
