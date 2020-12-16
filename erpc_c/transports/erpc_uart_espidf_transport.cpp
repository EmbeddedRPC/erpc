/*
 * Copyright (c) 2020 Sierra Wireless
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_uart_espidf_transport.h"
#include <cstdio>

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

static volatile bool s_isTransferReceiveCompleted = false;
static volatile bool s_isTransferSendCompleted = false;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

UartTransport::UartTransport(uart_config_t *uart_config, uart_port_t uart_num, gpio_num_t rx_gpio,
                             gpio_num_t tx_gpio, gpio_num_t rts_gpio, gpio_num_t cts_gpio)
: m_uart{
        .config = *uart_config,
        .num = (uart_port_t) uart_num,
        .rx_gpio = rx_gpio,
        .tx_gpio = tx_gpio,
        .rts_gpio = rts_gpio,
        .cts_gpio = cts_gpio,
        .rx_ring_size = 256,
        .tx_ring_size = 0,
        .queue_size = 0,
        .queue = NULL,
        .intr_alloc_flags = 0,
    }
{
}

UartTransport::~UartTransport(void)
{
    if (uart_is_driver_installed(m_uart.num))
    {
        uart_driver_delete(m_uart.num);
    }
}

erpc_status_t UartTransport::init(int timeout)
{
    // timeout is at least 2 ticks and 2x duration based on baudrate
    m_timeout = (timeout < 0) ? portMAX_DELAY : (timeout / portTICK_RATE_MS + 2);

    if (uart_is_driver_installed(m_uart.num))
    {
        uart_driver_delete(m_uart.num);
    }

    esp_err_t err;
    err = uart_param_config(m_uart.num, &m_uart.config);
    if (err != ESP_OK) {
        return kErpcStatus_InitFailed;
    }

    err = uart_set_pin(m_uart.num, m_uart.tx_gpio, m_uart.rx_gpio, m_uart.rts_gpio, m_uart.cts_gpio);
    if (err != ESP_OK) {
        return kErpcStatus_InitFailed;
    }

    err = uart_driver_install(
        m_uart.num,
        m_uart.rx_ring_size,
        m_uart.tx_ring_size,
        m_uart.queue_size,
        m_uart.queue,
        m_uart.intr_alloc_flags);
    if (err != ESP_OK) {
        return kErpcStatus_InitFailed;
    }

    return ESP_OK;
}

erpc_status_t UartTransport::underlyingReceive(uint8_t *data, uint32_t size)
{
    int len = -1;

    TickType_t timeout = (size * 10 * 1000 * 2) / m_uart.config.baud_rate / portTICK_RATE_MS;

    while (size &&
           ((len = uart_read_bytes(
                 m_uart.num, data, size, timeout < m_timeout ? m_timeout : timeout)) > 0))
    {
        size -= len;
    }

    if (size)
    {
        return (len == -1) ? kErpcStatus_ReceiveFailed : kErpcStatus_Timeout;
    }

    return kErpcStatus_Success;
}

erpc_status_t UartTransport::underlyingSend(const uint8_t *data, uint32_t size)
{
    int sent = uart_write_bytes(m_uart.num, (const char*) data, size);

    // not sure we need to do that, especially when TX fifo is 0
    if (sent != -1)
    {
        TickType_t timeout = (size * 10 * 1000 * 2) / m_uart.config.baud_rate / portTICK_RATE_MS;

        if (uart_wait_tx_done(m_uart.num, timeout < m_timeout ? m_timeout : timeout) == ESP_OK)
        {
            return kErpcStatus_Success;
        }
    }

    return kErpcStatus_SendFailed;
}

bool UartTransport::hasMessage(void)
{
    /*
     * It's not possible to know whether we truly have a message without reading a couple of bytes
     * from the UART that indicate the total size of the message beyond the 4-byte header and it is
     * inconvenient to do so because higher level code in FramedTransport will call
     * underlyingReceive and expect to get those first two bytes that indicate the size of the
     * message.
     *
     * Since it's not easy for us to know whether we truly have a message, we return true in the
     * case that we have at least 4 bytes (the size of the message header).
     */
    size_t buffered_data_len;
    if (uart_get_buffered_data_len(m_uart.num, &buffered_data_len) == ESP_OK)
    {
        return buffered_data_len >= sizeof(FramedTransport::Header);
    }

    return false;
}
