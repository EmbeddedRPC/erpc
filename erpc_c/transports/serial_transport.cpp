/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
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

#include "serial_transport.h"
#include "message_buffer.h"
#include "serial.h"
#include <cassert>
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

SerialTransport::~SerialTransport()
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
