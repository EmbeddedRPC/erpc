/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_dspi_slave_transport.h"
#include "board.h"
#include "fsl_dspi.h"
#include "fsl_gpio.h"
#include <cstdio>

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

static dspi_slave_handle_t g_s_handle;
static volatile bool s_isTransferCompleted = false;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

void DSPI_SlaveUserCallback(SPI_Type *base, dspi_slave_handle_t *handle, erpc_status_t status, void *userData)
{
    GPIO_PortSet(ERPC_BOARD_DSPI_INT_GPIO, 1U << ERPC_BOARD_DSPI_INT_PIN);
    s_isTransferCompleted = true;
}

DspiSlaveTransport::DspiSlaveTransport(SPI_Type *spiBaseAddr, uint32_t baudRate, uint32_t srcClock_Hz)
: m_spiBaseAddr(spiBaseAddr)
, m_baudRate(baudRate)
, m_srcClock_Hz(srcClock_Hz)
, m_isInited(false)
{
}

DspiSlaveTransport::~DspiSlaveTransport(void)
{
    if (m_isInited)
    {
        GPIO_PortClear(ERPC_BOARD_DSPI_INT_GPIO, 1U << ERPC_BOARD_DSPI_INT_PIN);
    }
    DSPI_Deinit(m_spiBaseAddr);
}

erpc_status_t DspiSlaveTransport::init(void)
{
    dspi_slave_config_t dspiConfig;
    gpio_pin_config_t gpioConfig;

    DSPI_SlaveGetDefaultConfig(&dspiConfig);

    DSPI_SlaveInit(m_spiBaseAddr, &dspiConfig);
    DSPI_SlaveTransferCreateHandle(m_spiBaseAddr, &g_s_handle, DSPI_SlaveUserCallback, NULL);

    gpioConfig.pinDirection = kGPIO_DigitalOutput;
    gpioConfig.outputLogic = 1U;

    GPIO_PinInit(ERPC_BOARD_DSPI_INT_GPIO, ERPC_BOARD_DSPI_INT_PIN, &gpioConfig);

    m_isInited = true;
    return kErpcStatus_Success;
}

erpc_status_t DspiSlaveTransport::underlyingReceive(uint8_t *data, uint32_t size)
{
    erpc_status_t status;
    dspi_transfer_t slaveXfer;

    slaveXfer.txData = NULL;
    slaveXfer.rxData = data;
    slaveXfer.dataSize = size;
    slaveXfer.configFlags = kDSPI_SlaveCtar0;
    s_isTransferCompleted = false;

    status = DSPI_SlaveTransferNonBlocking(m_spiBaseAddr, &g_s_handle, &slaveXfer);

    GPIO_PortClear(ERPC_BOARD_DSPI_INT_GPIO, 1U << ERPC_BOARD_DSPI_INT_PIN);
    while (!s_isTransferCompleted)
    {
    }

    return status != kStatus_Success ? kErpcStatus_ReceiveFailed : kErpcStatus_Success;
}

erpc_status_t DspiSlaveTransport::underlyingSend(const uint8_t *data, uint32_t size)
{
    erpc_status_t status;
    dspi_transfer_t slaveXfer;

    slaveXfer.txData = (uint8_t *)data;
    slaveXfer.rxData = NULL;
    slaveXfer.dataSize = size;
    slaveXfer.configFlags = kDSPI_SlaveCtar0;
    s_isTransferCompleted = false;

    status = DSPI_SlaveTransferNonBlocking(m_spiBaseAddr, &g_s_handle, &slaveXfer);

    GPIO_PortClear(ERPC_BOARD_DSPI_INT_GPIO, 1U << ERPC_BOARD_DSPI_INT_PIN);
    while (!s_isTransferCompleted)
    {
    }

    return status != kStatus_Success ? kErpcStatus_SendFailed : kErpcStatus_Success;
}
