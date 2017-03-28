/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
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
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
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

#include <cassert>
#include <cstdio>
#include "dspi_slave_transport.h"
#include "fsl_dspi.h"
#include "fsl_gpio.h"
#include "board.h"

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
    GPIO_SetPinsOutput(ERPC_BOARD_DSPI_INT_GPIO, 1U << ERPC_BOARD_DSPI_INT_PIN);
    s_isTransferCompleted = true;
}

DspiSlaveTransport::DspiSlaveTransport(SPI_Type *spiBaseAddr, uint32_t baudRate, uint32_t srcClock_Hz)
: m_spiBaseAddr(spiBaseAddr)
, m_baudRate(baudRate)
, m_srcClock_Hz(srcClock_Hz)
{
}

DspiSlaveTransport::~DspiSlaveTransport()
{
    DSPI_Deinit(m_spiBaseAddr);
}

erpc_status_t DspiSlaveTransport::init()
{
    dspi_slave_config_t dspiConfig;
    gpio_pin_config_t gpioConfig;

    DSPI_SlaveGetDefaultConfig(&dspiConfig);

    DSPI_SlaveInit(m_spiBaseAddr, &dspiConfig);
    DSPI_SlaveTransferCreateHandle(m_spiBaseAddr, &g_s_handle, DSPI_SlaveUserCallback, NULL);

    gpioConfig.pinDirection = kGPIO_DigitalOutput;
    gpioConfig.outputLogic = 1U;

    GPIO_PinInit(ERPC_BOARD_DSPI_INT_GPIO, ERPC_BOARD_DSPI_INT_PIN, &gpioConfig);

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

    GPIO_ClearPinsOutput(ERPC_BOARD_DSPI_INT_GPIO, 1U << ERPC_BOARD_DSPI_INT_PIN);
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

    GPIO_ClearPinsOutput(ERPC_BOARD_DSPI_INT_GPIO, 1U << ERPC_BOARD_DSPI_INT_PIN);
    while (!s_isTransferCompleted)
    {
    }

    return status != kStatus_Success ? kErpcStatus_SendFailed : kErpcStatus_Success;
}
