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
#include "uart_cmsis_transport.h"
#include "board.h"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

static volatile bool s_isTransferCompleted = false;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

UartTransport::UartTransport(ARM_DRIVER_USART *uartDrv)
: m_uartDrv(uartDrv)
{
}

UartTransport::~UartTransport()
{
    (*m_uartDrv).Uninitialize();
}

/* Transfer callback */
void TransferCallback(uint32_t event)
{
    if (event == ARM_USART_EVENT_SEND_COMPLETE)
    {
        s_isTransferCompleted = true;
    }

    if (event == ARM_USART_EVENT_RECEIVE_COMPLETE)
    {
        s_isTransferCompleted = true;
    }
}

erpc_status_t UartTransport::init()
{
    erpc_status_t status = (*m_uartDrv).Initialize(TransferCallback);
    (*m_uartDrv).PowerControl(ARM_POWER_FULL);

    return status != kStatus_Success ? kErpcStatus_InitFailed : kErpcStatus_Success;
}

erpc_status_t UartTransport::underlyingReceive(uint8_t *data, uint32_t size)
{
    s_isTransferCompleted = false;

    erpc_status_t status = (*m_uartDrv).Receive(data, size);
    while (!s_isTransferCompleted)
    {
    }

    return status != kStatus_Success ? kErpcStatus_ReceiveFailed : kErpcStatus_Success;
}

erpc_status_t UartTransport::underlyingSend(const uint8_t *data, uint32_t size)
{
    s_isTransferCompleted = false;

    (*m_uartDrv).Send(data, size);
    while (!s_isTransferCompleted)
    {
    }

    return kErpcStatus_Success;
}
