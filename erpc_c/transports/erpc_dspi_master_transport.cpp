/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_dspi_master_transport.hpp"

#include "board.h"
#include "fsl_dspi.h"
#include "fsl_gpio.h"
#include "fsl_port.h"

#include <cstdio>

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

#ifndef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
#define ERPC_BOARD_SPI_SLAVE_READY_MARKER1 0xABU
#define ERPC_BOARD_SPI_SLAVE_READY_MARKER2 0xCDU
#else
#ifndef ERPC_BOARD_DSPI_INT_GPIO
#error "Please define the ERPC_BOARD_DSPI_INT_GPIO used to notify when the DSPI Slave is ready to transmit"
#endif
#endif

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

static volatile bool s_isSlaveReady = false;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
/* @brief Initialize the GPIO used to notify the SPI Master */
static inline void DSpiMasterTransport_NotifyTransferGpioInit(void)
{
    gpio_pin_config_t gpioConfig;

    gpioConfig.pinDirection = kGPIO_DigitalInput;

    PORT_SetPinInterruptConfig(ERPC_BOARD_DSPI_INT_PORT, ERPC_BOARD_DSPI_INT_PIN, kPORT_InterruptFallingEdge);
    (void)EnableIRQ(ERPC_BOARD_DSPI_INT_PIN_IRQ);

    GPIO_PinInit(ERPC_BOARD_DSPI_INT_GPIO, ERPC_BOARD_DSPI_INT_PIN, &gpioConfig);
    if (0U == GPIO_PinRead(ERPC_BOARD_DSPI_INT_GPIO, ERPC_BOARD_DSPI_INT_PIN))
    {
        s_isSlaveReady = true;
    }
}

static inline void DSpidevMasterTransport_WaitForSlaveReadyGpio(void)
{
    while (!s_isSlaveReady)
    {
    }
}
#else
static inline void DSpidevMasterTransport_WaitForSlaveReadyMarker(SPI_Type *spiBaseAddr)
{
    uint8_t detected = 0;
    uint8_t data;
    dspi_transfer_t masterXferSlaveReadyMarker;

    for (;;)
    {
        masterXferSlaveReadyMarker.txData = NULL;
        masterXferSlaveReadyMarker.rxData = &data;
        masterXferSlaveReadyMarker.dataSize = 1;
        masterXferSlaveReadyMarker.configFlags = (uint32_t)kDSPI_MasterCtar0 | (uint32_t)kDSPI_MasterPcs0;

        if (kStatus_Success == DSPI_MasterTransferBlocking(spiBaseAddr, &masterXferSlaveReadyMarker))
        {
            if (ERPC_BOARD_SPI_SLAVE_READY_MARKER1 == data)
            {
                detected = 1;
            }
            else if (detected && (ERPC_BOARD_SPI_SLAVE_READY_MARKER2 == data))
            {
                break;
            }
            else
            {
                detected = 0;
                // Thread::sleep(100);
            }
        }
    }
}
#endif

DspiMasterTransport::DspiMasterTransport(SPI_Type *spiBaseAddr, uint32_t baudRate, uint32_t srcClock_Hz)
: m_spiBaseAddr(spiBaseAddr)
, m_baudRate(baudRate)
, m_srcClock_Hz(srcClock_Hz)
{
}

DspiMasterTransport::~DspiMasterTransport(void)
{
    DSPI_Deinit(m_spiBaseAddr);
}

erpc_status_t DspiMasterTransport::init(void)
{
    dspi_master_config_t dspiConfig;

    DSPI_MasterGetDefaultConfig(&dspiConfig);
    dspiConfig.ctarConfig.baudRate = m_baudRate;
    DSPI_MasterInit(m_spiBaseAddr, &dspiConfig, m_srcClock_Hz);

#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
    DSpiMasterTransport_NotifyTransferGpioInit();
#endif

    return kErpcStatus_Success;
}

erpc_status_t DspiMasterTransport::underlyingReceive(uint8_t *data, uint32_t size)
{
    status_t status;
    dspi_transfer_t masterXfer;

    masterXfer.txData = NULL;
    masterXfer.rxData = data;
    masterXfer.dataSize = size;
    masterXfer.configFlags = (uint32_t)kDSPI_MasterCtar0 | (uint32_t)kDSPI_MasterPcs0;

#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
    DSpidevMasterTransport_WaitForSlaveReadyGpio();
#else
    DSpidevMasterTransport_WaitForSlaveReadyMarker(m_spiBaseAddr);
#endif

    status = DSPI_MasterTransferBlocking(m_spiBaseAddr, &masterXfer);
#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
    s_isSlaveReady = false;
#endif

    return (status != kStatus_Success) ? kErpcStatus_ReceiveFailed : kErpcStatus_Success;
}

erpc_status_t DspiMasterTransport::underlyingSend(const uint8_t *data, uint32_t size)
{
    status_t status;
    dspi_transfer_t masterXfer;

    masterXfer.txData = (uint8_t *)data;
    masterXfer.rxData = NULL;
    masterXfer.dataSize = size;
    masterXfer.configFlags = (uint32_t)kDSPI_MasterCtar0 | (uint32_t)kDSPI_MasterPcs0;

#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
    DSpidevMasterTransport_WaitForSlaveReadyGpio();
#endif

    status = DSPI_MasterTransferBlocking(m_spiBaseAddr, &masterXfer);
#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
    s_isSlaveReady = false;
#endif

    return (status != kStatus_Success) ? kErpcStatus_SendFailed : kErpcStatus_Success;
}

#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
extern "C" {
void ERPC_BOARD_DSPI_INT_PIN_IRQ_HANDLER(void)
{
    /* Clear external interrupt flag. */
    GPIO_PortClearInterruptFlags(ERPC_BOARD_DSPI_INT_GPIO, 1U << ERPC_BOARD_DSPI_INT_PIN);
    s_isSlaveReady = true;
}
}
#endif
