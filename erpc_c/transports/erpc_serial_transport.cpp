/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_serial_transport.h"

#include "erpc_message_buffer.h"
#include "erpc_serial.h"

#include <cstdio>
#include <string>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <windows.h>
#elif defined(STM32F446xx)
    /* code need to be added for uart */
#else
#include <termios.h>
#endif

using namespace erpc;

UART_HandleTypeDef UARTHandle;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

#if defined(STM32F446xx)
SerialTransport::SerialTransport(USART_TypeDef *portName, speed_t baudRate)
#else
SerialTransport::SerialTransport(const char *portName, speed_t baudRate)
#endif
: m_serialHandle(NULL)
, m_portName(portName)
, m_baudRate(baudRate)
{
	m_serialHandle = &UARTHandle;
}

SerialTransport::~SerialTransport(void)
{
    serial_close(m_serialHandle);
}

erpc_status_t SerialTransport::init(uint8_t vtime, uint8_t vmin)
{
    erpc_status_t status = kErpcStatus_Success;

#if defined(STM32F446xx)
    int res = serial_open(m_portName, m_serialHandle);
    if (res == 0)
	{
		if (HAL_OK != serial_setup(m_serialHandle, m_baudRate))
		{
			status = kErpcStatus_InitFailed;
		}
	}
    else
    {
    	status = kErpcStatus_InitFailed;
    }
#else
    m_serialHandle = serial_open(m_portName);
    if (-1 == m_serialHandle)
    {
        status = kErpcStatus_InitFailed;
    }
#endif
#ifdef _WIN32
    // TODO
#elif defined(STM32F446xx)
    /* code need to be added for uart */
#else
    if (status == kErpcStatus_Success)
    {
        if (!isatty(m_serialHandle))
        {
            status = kErpcStatus_InitFailed;
        }
    }

    if (status == kErpcStatus_Success)
    {
        if (-1 == serial_setup(m_serialHandle, m_baudRate))
        {
            status = kErpcStatus_InitFailed;
        }
    }

    if (status == kErpcStatus_Success)
    {
        if (-1 == serial_set_read_timeout(m_serialHandle, vtime, vmin))
        {
            status = kErpcStatus_InitFailed;
        }
    }
#endif
#ifdef _WIN32
    // TODO
#elif defined(STM32F446xx)

#else
    if (status == kErpcStatus_Success)
    {
        if (-1 == tcflush(m_serialHandle, TCIOFLUSH))
        {
            status = kErpcStatus_InitFailed;
        }
    }
#endif

    return status;
}

erpc_status_t SerialTransport::underlyingSend(const uint8_t *data, uint32_t size)
{
#if defined(STM32F446xx)
	HAL_StatusTypeDef status = serial_write(m_serialHandle, (char *)data, size);
	return (status != HAL_OK) ? kErpcStatus_SendFailed : kErpcStatus_Success;
#else
    uint32_t bytesWritten = serial_write(m_serialHandle, (char *)data, size);

    return (size != bytesWritten) ? kErpcStatus_SendFailed : kErpcStatus_Success;
#endif
}
erpc_status_t SerialTransport::underlyingReceive(uint8_t *data, uint32_t size)
{
#if defined(STM32F446xx)
	HAL_StatusTypeDef status = serial_read(m_serialHandle, (char *)data, size);
	return (status != HAL_OK) ? kErpcStatus_SendFailed : kErpcStatus_Success;
#else
    uint32_t bytesRead = serial_read(m_serialHandle, (char *)data, size);

    return (size != bytesRead) ? kErpcStatus_ReceiveFailed : kErpcStatus_Success;
#endif
}
