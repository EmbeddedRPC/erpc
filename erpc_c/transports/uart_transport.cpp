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
extern "C" {
#include "uart_transport.h"
#include "board.h"
}
#include <cassert>
#include <cstdio>
#include <string>

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

UartTransport::UartTransport(UART_Type *uartBaseAddr, uint32_t baudRate, uint32_t srcClock_Hz)
: m_uartBaseAddr(uartBaseAddr)
, m_baudRate(baudRate)
, m_srcClock_Hz(srcClock_Hz)
{
}

UartTransport::~UartTransport()
{
    UART_Deinit(m_uartBaseAddr);
}

status_t UartTransport::init()
{
    UART_GetDefaultConfig(&m_uartConfig);
    /* Configure the UART for 8 data bits, No parity, and one stop bit
       The baudrate speed is set in board.h via BOARD_DEBUG_UART_BAUD macro */
    m_uartConfig.baudRate_Bps = m_baudRate;
    m_uartConfig.parityMode = kUART_ParityDisabled;
    m_uartConfig.enableTx = true;
    m_uartConfig.enableRx = true;

    /* Initialize the UART module */
    return UART_Init(m_uartBaseAddr, &m_uartConfig, m_srcClock_Hz);
}

status_t UartTransport::underlyingReceive(uint8_t *data, uint32_t size)
{
    return UART_ReadBlocking(m_uartBaseAddr, data, size);
}

status_t UartTransport::underlyingSend(const uint8_t *data, uint32_t size)
{
    UART_WriteBlocking(m_uartBaseAddr, data, size);
    return kErpcStatus_Success;
}
