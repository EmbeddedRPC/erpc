/*
 * Copyright (c) 2020 Sierra Wireless
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_spi_slave_espidf_transport.h"
#include "erpc_port.h"
#include <cstdio>
#include "driver/gpio.h"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

/* BEWARE: the esp32 spi in slave mode is very buggy, whether you use DMA or not
 * so this code is barely usable and use non-DMA.
 * - always a rx and tx buffer pointing to DMA memory, can't set one to NULL
 * - DMA requires %4 transfer size and 32-bits aligned memory (not practicle)
 * - non-DMA maximum transfer is 64 bytes (unacceptable limitation)
 * - non-DMA requires a CS gpio to work and anyway in esp-idf 4.0 setting CS to -1 crashes
 * - severe speed limitations (below 1MHz - to be confirmed)
 * - does not seem to recover when started after the master (in eRPC)
 */
#define MAX_SIZE    64
static DMA_ATTR uint8_t dummy[MAX_SIZE];

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

#define SET_GPIO(g,v)            \
    do {                         \
        if (g != GPIO_NUM_NC)    \
        {                        \
            gpio_set_level(g,v); \
        }                        \
    } while (0)

SpiSlaveTransport::SpiSlaveTransport(
    spi_bus_config_t *bus_config, spi_host_device_t host, gpio_num_t cs_gpio, gpio_num_t ready_gpio)
: m_spi{
        .bus_config = *bus_config,
        .config = { },
        .host = host,
    }
, m_ready_gpio(ready_gpio)
{
    m_spi.config.spics_io_num = cs_gpio;
    m_spi.config.queue_size = 1;
    m_spi.config.mode = 0;
}

SpiSlaveTransport::~SpiSlaveTransport(void)
{
    spi_slave_free(m_spi.host);
}

erpc_status_t SpiSlaveTransport::init(int timeout)
{
    m_timeout = (timeout < 0) ? portMAX_DELAY : (timeout / portTICK_PERIOD_MS);

    if (m_ready_gpio != GPIO_NUM_NC)
    {
        gpio_config_t gpio_conf = {
            .pin_bit_mask = (1ULL << m_ready_gpio),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };

        gpio_config(&gpio_conf);
        gpio_set_level(m_ready_gpio, 0);
    }

    if (spi_slave_initialize(m_spi.host, &m_spi.bus_config, &m_spi.config, 1) != ESP_OK)
    {
        return kErpcStatus_InitFailed;
    }

    return kErpcStatus_Success;
}

erpc_status_t SpiSlaveTransport::underlyingReceive(uint8_t *data, uint32_t size)
{
    spi_slave_transaction_t transaction = { }, *result;

    if (size > MAX_SIZE)
    {
        return kErpcStatus_InvalidArgument;
    }

    transaction.length = size * 8;
    transaction.rx_buffer = data;
    transaction.tx_buffer = dummy;

    if (spi_slave_queue_trans(m_spi.host, &transaction, m_timeout) != ESP_OK)
    {
        return kErpcStatus_ReceiveFailed;
    }

    SET_GPIO(m_ready_gpio, 1);
    if (spi_slave_get_trans_result(m_spi.host, &result, m_timeout) != ESP_OK)
    {
        SET_GPIO(m_ready_gpio, 0);
        return kErpcStatus_Timeout;
    }
    SET_GPIO(m_ready_gpio, 0);

    return kErpcStatus_Success;
}

erpc_status_t SpiSlaveTransport::underlyingSend(const uint8_t *data, uint32_t size)
{
    spi_slave_transaction_t transaction = { }, *result;

    if (size > MAX_SIZE)
    {
        return kErpcStatus_InvalidArgument;
    }

    transaction.length = size * 8;
    transaction.tx_buffer = (void*) data;
    transaction.rx_buffer = dummy;

    if (spi_slave_queue_trans(m_spi.host, &transaction, m_timeout) != ESP_OK)
    {
        return kErpcStatus_SendFailed;
    }

    SET_GPIO(m_ready_gpio, 1);
    if (spi_slave_get_trans_result(m_spi.host, &result, m_timeout) != ESP_OK)
    {
        SET_GPIO(m_ready_gpio, 0);
        return kErpcStatus_Timeout;
    }
    SET_GPIO(m_ready_gpio, 0);

    return kErpcStatus_Success;
}
