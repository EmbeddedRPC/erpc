/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
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
#include <new>

using namespace std;
using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////
#ifndef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
#define ERPC_BOARD_SPI_SLAVE_READY_MARKER_LEN 2
#define ERPC_BOARD_SPI_SLAVE_READY_MARKER1 0xAB
#define ERPC_BOARD_SPI_SLAVE_READY_MARKER2 0xCD
#else
#ifndef ERPC_BOARD_DSPI_INT_GPIO
#error "Please define the ERPC_BOARD_DSPI_INT_GPIO used to notify when the DSPI Slave is ready to transmit"
#endif
#endif

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

static dspi_slave_handle_t g_s_handle;
static volatile bool s_isTransferCompleted = false;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
/* @brief Initialize the GPIO used to notify the SPI Master */
static inline void DSpiSlaveTransport_NotifyTransferGpioInit(void)
{
    gpio_pin_config_t gpioConfig;

    gpioConfig.pinDirection = kGPIO_DigitalOutput;
    gpioConfig.outputLogic = 1U;

    GPIO_PinInit(ERPC_BOARD_DSPI_INT_GPIO, ERPC_BOARD_DSPI_INT_PIN, &gpioConfig);
}

/* @brief Notify the SPI Master that the Slave is ready for a new transfer */
static inline void DSpiSlaveTransport_NotifyTransferGpioReady(void)
{
    GPIO_PortClear(ERPC_BOARD_DSPI_INT_GPIO, 1U << ERPC_BOARD_DSPI_INT_PIN);
}

/* @brief Notify the SPI Master that the Slave has finished the transfer */
static inline void DSpiSlaveTransport_NotifyTransferGpioCompleted(void)
{
    (void)base;
    (void)handle;
    (void)status;
    (void)userData;

    GPIO_PortSet(ERPC_BOARD_DSPI_INT_GPIO, 1U << ERPC_BOARD_DSPI_INT_PIN);
}
#endif

static void DSPI_SlaveUserCallback(SPI_Type *base, dspi_slave_handle_t *handle, erpc_status_t status, void *userData)
{
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
#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
        DSpiSlaveTransport_NotifyTransferGpioCompleted();
#endif
        DSPI_Deinit(m_spiBaseAddr);
        m_isInited = false;
    }
}

erpc_status_t DspiSlaveTransport::init(void)
{
    dspi_slave_config_t dspiConfig;

    DSPI_SlaveGetDefaultConfig(&dspiConfig);

    DSPI_SlaveInit(m_spiBaseAddr, &dspiConfig);
    DSPI_SlaveTransferCreateHandle(m_spiBaseAddr, &g_s_handle, DSPI_SlaveUserCallback, NULL);

#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
    DSpiSlaveTransport_NotifyTransferGpioInit();
#endif

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

    if (kStatus_Success == status)
    {
#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
        DSpiSlaveTransport_NotifyTransferGpioReady();
#endif
        while (!s_isTransferCompleted)
        {
        }
#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
        DSpiSlaveTransport_NotifyTransferGpioCompleted();
#endif
    }

    return (status != kStatus_Success) ? kErpcStatus_ReceiveFailed : kErpcStatus_Success;
}

erpc_status_t DspiSlaveTransport::underlyingSend(const uint8_t *data, uint32_t size)
{
    erpc_status_t status;
    dspi_transfer_t slaveXfer;
    s_isTransferCompleted = false;

#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
    slaveXfer.txData = (uint8_t *)data;
    slaveXfer.rxData = NULL;
    slaveXfer.dataSize = size;
    slaveXfer.configFlags = kDSPI_SlaveCtar0;
#else
    uint8_t *dspiData = new (nothrow) uint8_t[size + ERPC_BOARD_SPI_SLAVE_READY_MARKER_LEN];
    if (dspiData != NULL)
    {
        dspiData[0] = ERPC_BOARD_SPI_SLAVE_READY_MARKER1;
        dspiData[1] = ERPC_BOARD_SPI_SLAVE_READY_MARKER2;
        memcpy(&dspiData[ERPC_BOARD_SPI_SLAVE_READY_MARKER_LEN], data, size);
    }
    else
    {
        return kErpcStatus_SendFailed;
    }

    slaveXfer.txData = dspiData;
    slaveXfer.rxData = NULL;
    slaveXfer.dataSize = size + ERPC_BOARD_SPI_SLAVE_READY_MARKER_LEN;
    slaveXfer.configFlags = kDSPI_SlaveCtar0;
#endif

    status = DSPI_SlaveTransferNonBlocking(m_spiBaseAddr, &g_s_handle, &slaveXfer);

    if (kStatus_Success == status)
    {
#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
        DSpiSlaveTransport_NotifyTransferGpioReady();
#endif
        while (!s_isTransferCompleted)
        {
        }
#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
        DSpiSlaveTransport_NotifyTransferGpioCompleted();
#endif
    }
#ifndef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
    delete[] dspiData;
#endif

    return (status != kStatus_Success) ? kErpcStatus_SendFailed : kErpcStatus_Success;
}
