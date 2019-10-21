/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_uart_cmsis_transport.h"
#include <cstdio>

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

static volatile bool s_isTransferReceiveCompleted = false;
static volatile bool s_isTransferSendCompleted = false;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

UartTransport::UartTransport(ARM_DRIVER_USART *uartDrv)
: m_uartDrv(uartDrv)
{
}

UartTransport::~UartTransport(void)
{
    (*m_uartDrv).Uninitialize();
}

/* Transfer callback */
void TransferCallback(uint32_t event)
{
    if (event == ARM_USART_EVENT_SEND_COMPLETE)
    {
        s_isTransferSendCompleted = true;
    }

    if (event == ARM_USART_EVENT_RECEIVE_COMPLETE)
    {
        s_isTransferReceiveCompleted = true;
    }
}

erpc_status_t UartTransport::init(void)
{
    int32_t status = (*m_uartDrv).Initialize(TransferCallback);
    if (status == ARM_DRIVER_OK)
    {
        status = (*m_uartDrv).PowerControl(ARM_POWER_FULL); /* Enable Receiver and Transmitter lines */
        if (status == ARM_DRIVER_OK)
        {
            status = m_uartDrv->Control(ARM_USART_CONTROL_TX, 1);
            if (status == ARM_DRIVER_OK)
            {
                status = m_uartDrv->Control(ARM_USART_CONTROL_RX, 1);
                if (status == ARM_DRIVER_OK)
                {
                    return kErpcStatus_Success;
                }
            }
        }
    }

    return kErpcStatus_InitFailed;
}

erpc_status_t UartTransport::underlyingReceive(uint8_t *data, uint32_t size)
{
    s_isTransferReceiveCompleted = false;

    int32_t status = (*m_uartDrv).Receive(data, size);
    if (status == ARM_DRIVER_OK)
    {
        while (!s_isTransferReceiveCompleted)
        {
        }
        return kErpcStatus_Success;
    }

    return kErpcStatus_ReceiveFailed;
}

erpc_status_t UartTransport::underlyingSend(const uint8_t *data, uint32_t size)
{
    s_isTransferSendCompleted = false;

    int32_t status = (*m_uartDrv).Send(data, size);
    if (status == ARM_DRIVER_OK)
    {
        while (!s_isTransferSendCompleted)
        {
        }
        return kErpcStatus_Success;
    }

    return kErpcStatus_SendFailed;
}
