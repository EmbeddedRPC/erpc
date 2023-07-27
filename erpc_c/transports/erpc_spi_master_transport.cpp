/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cstdio>
#include "erpc_spi_master_transport.hpp"

extern "C" {
#include "board.h"
#include "fsl_gpio.h"
#include "fsl_port.h"
#include "fsl_spi.h"
}

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

#ifndef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
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

static volatile bool s_isSlaveReady = false;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
/* @brief Initialize the GPIO used to notify the SPI Master */
static inline void SpiMasterTransport_NotifyTransferGpioInit()
{
    gpio_pin_config_t gpioConfig;

    gpioConfig.pinDirection = kGPIO_DigitalInput;

    PORT_SetPinInterruptConfig(ERPC_BOARD_SPI_INT_PORT, ERPC_BOARD_SPI_INT_PIN, kPORT_InterruptFallingEdge);
    (void)EnableIRQ(ERPC_BOARD_SPI_INT_PIN_IRQ);

    GPIO_PinInit(ERPC_BOARD_SPI_INT_GPIO, ERPC_BOARD_SPI_INT_PIN, &gpioConfig);
    if (0U == GPIO_PinRead(ERPC_BOARD_SPI_INT_GPIO, ERPC_BOARD_SPI_INT_PIN))
    {
        s_isSlaveReady = true;
    }
}

static inline void SpidevMasterTransport_WaitForSlaveReadyGpio()
{
    while (!s_isSlaveReady)
    {
    }
}
#else
static inline void SpidevMasterTransport_WaitForSlaveReadyMarker(SPI_Type *spiBaseAddr)
{
    uint8_t detected = 0;
    uint8_t data;
    spi_transfer_t masterXferSlaveReadyMarker;

    for (;;)
    {
        masterXferSlaveReadyMarker.txData = NULL;
        masterXferSlaveReadyMarker.rxData = &data;
        masterXferSlaveReadyMarker.dataSize = 1;

        if (kStatus_Success == SPI_MasterTransferBlocking(spiBaseAddr, &masterXferSlaveReadyMarker))
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

SpiMasterTransport::SpiMasterTransport(SPI_Type *spiBaseAddr, uint32_t baudRate, uint32_t srcClock_Hz)
: m_spiBaseAddr(spiBaseAddr)
, m_baudRate(baudRate)
, m_srcClock_Hz(srcClock_Hz)
{
}

SpiMasterTransport::~SpiMasterTransport(void)
{
    SPI_Deinit(m_spiBaseAddr);
}

erpc_status_t SpiMasterTransport::init(void)
{
    spi_master_config_t spiConfig;

    SPI_MasterGetDefaultConfig(&spiConfig);
    spiConfig.baudRate_Bps = m_baudRate;
    SPI_MasterInit(m_spiBaseAddr, &spiConfig, m_srcClock_Hz);

#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
    SpiMasterTransport_NotifyTransferGpioInit();
#endif

    return kErpcStatus_Success;
}

erpc_status_t SpiMasterTransport::underlyingReceive(uint8_t *data, uint32_t size)
{
    status_t status;
    spi_transfer_t masterXfer;

    masterXfer.txData = NULL;
    masterXfer.rxData = data;
    masterXfer.dataSize = size;

#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
    SpidevMasterTransport_WaitForSlaveReadyGpio();
#else
    SpidevMasterTransport_WaitForSlaveReadyMarker(m_spiBaseAddr);
#endif

    status = SPI_MasterTransferBlocking(m_spiBaseAddr, &masterXfer);
#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
    s_isSlaveReady = false;
#endif

    return (status != kStatus_Success) ? kErpcStatus_ReceiveFailed : kErpcStatus_Success;
}

erpc_status_t SpiMasterTransport::underlyingSend(const uint8_t *data, uint32_t size)
{
    status_t status;
    spi_transfer_t masterXfer;

    masterXfer.txData = (uint8_t *)data;
    masterXfer.rxData = NULL;
    masterXfer.dataSize = size;

#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
    SpidevMasterTransport_WaitForSlaveReadyGpio();
#endif

    status = SPI_MasterTransferBlocking(m_spiBaseAddr, &masterXfer);
#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
    s_isSlaveReady = false;
#endif

    return (status != kStatus_Success) ? kErpcStatus_SendFailed : kErpcStatus_Success;
}

#ifdef ERPC_BOARD_SPI_SLAVE_READY_USE_GPIO
extern "C" {
void ERPC_BOARD_SPI_INT_PIN_IRQ_HANDLER(void)
{
    /* Clear external interrupt flag. */
    GPIO_PortClearInterruptFlags(ERPC_BOARD_SPI_INT_GPIO, 1U << ERPC_BOARD_SPI_INT_PIN);
    s_isSlaveReady = true;
}
}
#endif
