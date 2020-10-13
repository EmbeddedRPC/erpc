/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
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
static UartTransport *s_uart_instance = NULL;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

UartTransport::UartTransport(ARM_DRIVER_USART *uartDrv)
: m_uartDrv(uartDrv)
#if ERPC_THREADS
, m_rxSemaphore()
, m_txSemaphore()
#endif
{
    s_uart_instance = this;
}

UartTransport::~UartTransport(void)
{
    (*m_uartDrv).Uninitialize();
}

void UartTransport::tx_cb(void)
{
#if ERPC_THREADS
    m_txSemaphore.putFromISR();
#else
    s_isTransferSendCompleted = true;
#endif
}

void UartTransport::rx_cb(void)
{
#if ERPC_THREADS
    m_rxSemaphore.putFromISR();
#else
    s_isTransferReceiveCompleted = true;
#endif
}

/* Transfer callback */
void TransferCallback(uint32_t event)
{
    UartTransport *transport = s_uart_instance;

    if (event == ARM_USART_EVENT_SEND_COMPLETE)
    {
        transport->tx_cb();
    }

    if (event == ARM_USART_EVENT_RECEIVE_COMPLETE)
    {
        transport->rx_cb();
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
/* wait until the receiving is finished */
#if ERPC_THREADS
        m_rxSemaphore.get();
#else
        while (!s_isTransferReceiveCompleted)
        {
        }
#endif
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
/* wait until the sending is finished */
#if ERPC_THREADS
        m_txSemaphore.get();
#else
        while (!s_isTransferSendCompleted)
        {
        }
#endif
        return kErpcStatus_Success;
    }

    return kErpcStatus_SendFailed;
}
