/*
 * Copyright (c) 2020 Sierra Wireless
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_spi_master_espidf_transport.h"
#include "erpc_port.h"
#include "driver/gpio.h"
#include <cstdio>

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

static void IRAM_ATTR handshakeHandler(void* arg)
{
    BaseType_t yield = false;
    xSemaphoreGiveFromISR((SemaphoreHandle_t) arg, &yield);
    if (yield) 
    {    
        portYIELD_FROM_ISR();
    }    
}

SpiMasterTransport::SpiMasterTransport(spi_bus_config_t *bus_config, spi_host_device_t host, gpio_num_t cs_gpio, gpio_num_t ready_gpio, int speed)
: m_spi{
        .bus_config = NULL,
        .config = { },
        .host = host,
        .handle = NULL,
    }
, m_ready_gpio(ready_gpio)    
{
    m_spi.config.clock_speed_hz = speed > 0 ? speed : SPI_MASTER_FREQ_8M;
    m_spi.config.spics_io_num = cs_gpio;
    m_spi.config.queue_size = 1;
    m_spi.config.mode = 0;
    
    if (bus_config)
    {
        m_spi.bus_config = (spi_bus_config_t*) erpc_malloc(sizeof(spi_bus_config_t));
        *m_spi.bus_config = *bus_config;
    }    
}

SpiMasterTransport::~SpiMasterTransport(void)
{
    spi_bus_remove_device(m_spi.handle);
    if (m_spi.bus_config)
    {
        spi_bus_free(m_spi.host);
        erpc_free(m_spi.bus_config);
    }
}

erpc_status_t SpiMasterTransport::init(int timeout)
{
    m_timeout = (timeout < 0) ? portMAX_DELAY : (timeout / portTICK_PERIOD_MS);

    if (m_ready_gpio != GPIO_NUM_NC) 
    {    
        m_ready_sem = xSemaphoreCreateBinary();
        
        gpio_config_t gpio_conf = { 
            .pin_bit_mask = (1ULL << m_ready_gpio),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_POSEDGE,
        };
        
        gpio_config(&gpio_conf);    
        gpio_install_isr_service(0);
        gpio_set_intr_type(m_ready_gpio, GPIO_INTR_POSEDGE);        
        gpio_isr_handler_add(m_ready_gpio, handshakeHandler, m_ready_sem);
        
        if (gpio_get_level(m_ready_gpio))
        {
            xSemaphoreGive(m_ready_sem);            
        }
    }

    if (m_spi.bus_config) 
    {
        if ((gpio_num_t) m_spi.config.spics_io_num != GPIO_NUM_NC) 
        {    
            // avoid spurious CS toggle when initializing bus
            gpio_set_level((gpio_num_t) m_spi.config.spics_io_num, 1);    
        }
        
        if (spi_bus_initialize( m_spi.host, m_spi.bus_config, 1) != ESP_OK) 
        {
            return kErpcStatus_InitFailed;
        }
    }    

    if (spi_bus_add_device(m_spi.host, &m_spi.config, &m_spi.handle) != ESP_OK) 
    {
        return kErpcStatus_InitFailed;
    }    

    return kErpcStatus_Success;
}

erpc_status_t SpiMasterTransport::underlyingReceive(uint8_t *data, uint32_t size)
{
    spi_transaction_t transaction = { };

    transaction.length = size * 8;    

    if (size <= 4) 
    {
        transaction.flags |= SPI_TRANS_USE_RXDATA;
    }
    else 
    {
        transaction.rx_buffer = data;    
    }        

    if (!xSemaphoreTake(m_ready_sem, m_timeout)) 
    {
        return kErpcStatus_Timeout;
    }    
    
    if (spi_device_polling_transmit(m_spi.handle, &transaction) != ESP_OK) 
    { 
        return kErpcStatus_ReceiveFailed;
    }
    
    while (size <= 4 && size--)
    {
        data[size] = transaction.rx_data[size];
    }

    return kErpcStatus_Success;
}

erpc_status_t SpiMasterTransport::underlyingSend(const uint8_t *data, uint32_t size)
{
    spi_transaction_t transaction = { };
    
    transaction.length = size * 8;    
        
    if (size <= 4) 
    {
        transaction.flags |= SPI_TRANS_USE_TXDATA;
        while (size--) 
        {
            transaction.tx_data[size] = data[size];
        }
    }
    else 
    {
        transaction.tx_buffer = data;    
    }        

    if (!xSemaphoreTake(m_ready_sem, m_timeout))
    {
        return kErpcStatus_Timeout;
    }    

    if (spi_device_polling_transmit(m_spi.handle, &transaction) != ESP_OK) 
    { 
        return kErpcStatus_SendFailed;
    }

    return kErpcStatus_Success;
}
