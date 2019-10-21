/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
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
#include <termios.h>

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

SerialTransport::SerialTransport(const char *portName, speed_t baudRate)
: m_serialHandle(0)
, m_portName(portName)
, m_baudRate(baudRate)
{
}

SerialTransport::~SerialTransport(void)
{
    serial_close(m_serialHandle);
}

erpc_status_t SerialTransport::init(uint8_t vtime, uint8_t vmin)
{
    m_serialHandle = serial_open(m_portName);
    if (-1 == m_serialHandle)
    {
        return kErpcStatus_InitFailed;
    }
    if (!isatty(m_serialHandle))
    {
        return kErpcStatus_InitFailed;
    }
    if (-1 == serial_setup(m_serialHandle, m_baudRate))
    {
        return kErpcStatus_InitFailed;
    }
    if (-1 == serial_set_read_timeout(m_serialHandle, vtime, vmin))
    {
        return kErpcStatus_InitFailed;
    }
    if (-1 == tcflush(m_serialHandle, TCIOFLUSH))
    {
        return kErpcStatus_InitFailed;
    }
    return kErpcStatus_Success;
}

erpc_status_t SerialTransport::underlyingSend(const uint8_t *data, uint32_t size)
{
    uint32_t bytesWritten = serial_write(m_serialHandle, (char *)data, size);

    return size != bytesWritten ? kErpcStatus_SendFailed : kErpcStatus_Success;
}
erpc_status_t SerialTransport::underlyingReceive(uint8_t *data, uint32_t size)
{
    uint32_t bytesRead = serial_read(m_serialHandle, (char *)data, size);

    return size != bytesRead ? kErpcStatus_ReceiveFailed : kErpcStatus_Success;
}
