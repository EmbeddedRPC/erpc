/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_spi_slave_transport.h"
#include "board.h"
#include "fsl_gpio.h"
#include "fsl_spi.h"
#include <cstdio>

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

static spi_slave_handle_t s_s_handle;
static volatile bool s_isTransferCompleted = false;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

void SPI_SlaveUserCallback(SPI_Type *base, spi_slave_handle_t *handle, erpc_status_t status, void *userData)
{
    GPIO_PortSet(ERPC_BOARD_SPI_INT_GPIO, 1U << ERPC_BOARD_SPI_INT_PIN);
    s_isTransferCompleted = true;
}

SpiSlaveTransport::SpiSlaveTransport(SPI_Type *spiBaseAddr, uint32_t baudRate, uint32_t srcClock_Hz)
: m_spiBaseAddr(spiBaseAddr)
, m_baudRate(baudRate)
, m_srcClock_Hz(srcClock_Hz)
, m_isInited(false)
{
}

SpiSlaveTransport::~SpiSlaveTransport(void)
{
    if (m_isInited)
    {
        GPIO_PortClear(ERPC_BOARD_SPI_INT_GPIO, 1U << ERPC_BOARD_SPI_INT_PIN);
    }
    SPI_Deinit(m_spiBaseAddr);
}

erpc_status_t SpiSlaveTransport::init(void)
{
    spi_slave_config_t spiConfig;
    gpio_pin_config_t gpioConfig;

    SPI_SlaveGetDefaultConfig(&spiConfig);

    SPI_SlaveInit(m_spiBaseAddr, &spiConfig);
    SPI_SlaveTransferCreateHandle(m_spiBaseAddr, &s_s_handle, SPI_SlaveUserCallback, NULL);

    gpioConfig.pinDirection = kGPIO_DigitalOutput;
    gpioConfig.outputLogic = 1U;

    GPIO_PinInit(ERPC_BOARD_SPI_INT_GPIO, ERPC_BOARD_SPI_INT_PIN, &gpioConfig);
    m_isInited = true;
    return kErpcStatus_Success;
}

erpc_status_t SpiSlaveTransport::underlyingReceive(uint8_t *data, uint32_t size)
{
    erpc_status_t status;
    spi_transfer_t slaveXfer;

    slaveXfer.txData = NULL;
    slaveXfer.rxData = data;
    slaveXfer.dataSize = size;
    s_isTransferCompleted = false;

    status = SPI_SlaveTransferNonBlocking(m_spiBaseAddr, &s_s_handle, &slaveXfer);

    GPIO_PortClear(ERPC_BOARD_SPI_INT_GPIO, 1U << ERPC_BOARD_SPI_INT_PIN);
    while (!s_isTransferCompleted)
    {
    }

    return status != kStatus_Success ? kErpcStatus_ReceiveFailed : kErpcStatus_Success;
}

erpc_status_t SpiSlaveTransport::underlyingSend(const uint8_t *data, uint32_t size)
{
    erpc_status_t status;
    spi_transfer_t slaveXfer;

    slaveXfer.txData = (uint8_t *)data;
    slaveXfer.rxData = NULL;
    slaveXfer.dataSize = size;
    s_isTransferCompleted = false;

    status = SPI_SlaveTransferNonBlocking(m_spiBaseAddr, &s_s_handle, &slaveXfer);

    GPIO_PortClear(ERPC_BOARD_SPI_INT_GPIO, 1U << ERPC_BOARD_SPI_INT_PIN);
    while (!s_isTransferCompleted)
    {
    }

    return status != kStatus_Success ? kErpcStatus_SendFailed : kErpcStatus_Success;
}
