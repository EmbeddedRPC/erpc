/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
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
#include <new>

using namespace std;
using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

#ifndef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
#define ERPC_BOARD_SPI_SLAVE_READY_MARKER_LEN 2U
#define ERPC_BOARD_SPI_SLAVE_READY_MARKER1 0xABU
#define ERPC_BOARD_SPI_SLAVE_READY_MARKER2 0xCDU
#else
#ifndef ERPC_BOARD_SPI_INT_GPIO
#error "Please define the ERPC_BOARD_SPI_INT_GPIO used to notify when the SPI Slave is ready to transmit"
#endif
#endif

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

static spi_slave_handle_t s_s_handle;
static volatile bool s_isTransferCompleted = false;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
/* @brief Initialize the GPIO used to notify the SPI Master */
static inline void SpiSlaveTransport_NotifyTransferGpioInit()
{
    gpio_pin_config_t gpioConfig;

    gpioConfig.pinDirection = kGPIO_DigitalOutput;
    gpioConfig.outputLogic = 1U;

    GPIO_PinInit(ERPC_BOARD_SPI_INT_GPIO, ERPC_BOARD_SPI_INT_PIN, &gpioConfig);
}

/* @brief Notify the SPI Master that the Slave is ready for a new transfer */
static inline void SpiSlaveTransport_NotifyTransferGpioReady()
{
    GPIO_PortClear(ERPC_BOARD_SPI_INT_GPIO, 1U << ERPC_BOARD_SPI_INT_PIN);
}

/* @brief Notify the SPI Master that the Slave has finished the transfer */
static inline void SpiSlaveTransport_NotifyTransferGpioCompleted()
{
    GPIO_PortSet(ERPC_BOARD_SPI_INT_GPIO, 1U << ERPC_BOARD_SPI_INT_PIN);
}
#endif

static void SPI_SlaveUserCallback(SPI_Type *base, spi_slave_handle_t *handle, erpc_status_t status, void *userData)
{
    (void)base;
    (void)handle;
    (void)status;
    (void)userData;

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
#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
        SpiSlaveTransport_NotifyTransferGpioCompleted();
#endif
        SPI_Deinit(m_spiBaseAddr);
        m_isInited = false;
    }
}

erpc_status_t SpiSlaveTransport::init(void)
{
    spi_slave_config_t spiConfig;

    SPI_SlaveGetDefaultConfig(&spiConfig);

    SPI_SlaveInit(m_spiBaseAddr, &spiConfig);
    SPI_SlaveTransferCreateHandle(m_spiBaseAddr, &s_s_handle, SPI_SlaveUserCallback, NULL);

#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
    SpiSlaveTransport_NotifyTransferGpioInit();
#endif

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

    if (kStatus_Success == status)
    {
#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
        SpiSlaveTransport_NotifyTransferGpioReady();
#endif
        while (!s_isTransferCompleted)
        {
        }
#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
        SpiSlaveTransport_NotifyTransferGpioCompleted();
#endif
    }

    return (status != kStatus_Success) ? kErpcStatus_ReceiveFailed : kErpcStatus_Success;
}

erpc_status_t SpiSlaveTransport::underlyingSend(const uint8_t *data, uint32_t size)
{
    erpc_status_t status;
    spi_transfer_t slaveXfer;
    s_isTransferCompleted = false;

#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
    slaveXfer.txData = (uint8_t *)data;
    slaveXfer.rxData = NULL;
    slaveXfer.dataSize = size;
    {
#else
    uint8_t *spiData = new (nothrow) uint8_t[size + ERPC_BOARD_SPI_SLAVE_READY_MARKER_LEN];
    if (spiData != NULL)
    {
        spiData[0] = ERPC_BOARD_SPI_SLAVE_READY_MARKER1;
        spiData[1] = ERPC_BOARD_SPI_SLAVE_READY_MARKER2;
        memcpy(&spiData[ERPC_BOARD_SPI_SLAVE_READY_MARKER_LEN], data, size);
        slaveXfer.txData = spiData;
        slaveXfer.rxData = NULL;
        slaveXfer.dataSize = size + ERPC_BOARD_SPI_SLAVE_READY_MARKER_LEN;
#endif

        status = SPI_SlaveTransferNonBlocking(m_spiBaseAddr, &s_s_handle, &slaveXfer);

        if (kStatus_Success == status)
        {
#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
            SpiSlaveTransport_NotifyTransferGpioReady();
#endif
            while (!s_isTransferCompleted)
            {
            }
#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
            SpiSlaveTransport_NotifyTransferGpioCompleted();
#endif
        }
#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
    }
#else ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
        delete[] spiData;
    }
    else
    {
        status = kErpcStatus_SendFailed;
    }
#endif

    return (status != kStatus_Success) ? kErpcStatus_SendFailed : kErpcStatus_Success;
}
