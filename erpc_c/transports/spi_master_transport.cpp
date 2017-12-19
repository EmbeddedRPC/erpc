/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "spi_master_transport.h"
#include "board.h"
#include "fsl_gpio.h"
#include "fsl_port.h"
#include "fsl_spi.h"
#include <cassert>
#include <cstdio>

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

static volatile bool s_isSlaveReady = false;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

SpiMasterTransport::SpiMasterTransport(SPI_Type *spiBaseAddr, uint32_t baudRate, uint32_t srcClock_Hz)
: m_spiBaseAddr(spiBaseAddr)
, m_baudRate(baudRate)
, m_srcClock_Hz(srcClock_Hz)
{
}

SpiMasterTransport::~SpiMasterTransport()
{
    SPI_Deinit(m_spiBaseAddr);
}

erpc_status_t SpiMasterTransport::init()
{
    spi_master_config_t spiConfig;
    gpio_pin_config_t gpioConfig;

    SPI_MasterGetDefaultConfig(&spiConfig);
    spiConfig.baudRate_Bps = m_baudRate;
    SPI_MasterInit(m_spiBaseAddr, &spiConfig, m_srcClock_Hz);

    gpioConfig.pinDirection = kGPIO_DigitalInput;

    PORT_SetPinInterruptConfig(ERPC_BOARD_SPI_INT_PORT, ERPC_BOARD_SPI_INT_PIN, kPORT_InterruptFallingEdge);
    EnableIRQ(ERPC_BOARD_SPI_INT_PIN_IRQ);

    GPIO_PinInit(ERPC_BOARD_SPI_INT_GPIO, ERPC_BOARD_SPI_INT_PIN, &gpioConfig);
    if (!GPIO_ReadPinInput(ERPC_BOARD_SPI_INT_GPIO, ERPC_BOARD_SPI_INT_PIN))
    {
        s_isSlaveReady = true;
    }

    return kErpcStatus_Success;
}

erpc_status_t SpiMasterTransport::underlyingReceive(uint8_t *data, uint32_t size)
{
    erpc_status_t status;
    spi_transfer_t masterXfer;

    masterXfer.txData = NULL;
    masterXfer.rxData = data;
    masterXfer.dataSize = size;

    while (!s_isSlaveReady)
    {
    }

    status = SPI_MasterTransferBlocking(m_spiBaseAddr, &masterXfer);
    s_isSlaveReady = false;

    return status != kStatus_Success ? kErpcStatus_ReceiveFailed : kErpcStatus_Success;
}

erpc_status_t SpiMasterTransport::underlyingSend(const uint8_t *data, uint32_t size)
{
    erpc_status_t status;
    spi_transfer_t masterXfer;

    masterXfer.txData = (uint8_t *)data;
    masterXfer.rxData = NULL;
    masterXfer.dataSize = size;

    while (!s_isSlaveReady)
    {
    }

    status = SPI_MasterTransferBlocking(m_spiBaseAddr, &masterXfer);
    s_isSlaveReady = false;

    return status != kStatus_Success ? kErpcStatus_SendFailed : kErpcStatus_Success;
}

extern "C" {
void ERPC_BOARD_SPI_INT_PIN_IRQ_HANDLER(void)
{
    /* Clear external interrupt flag. */
    GPIO_ClearPinsInterruptFlags(ERPC_BOARD_SPI_INT_GPIO, 1U << ERPC_BOARD_SPI_INT_PIN);
    s_isSlaveReady = true;
}
}
