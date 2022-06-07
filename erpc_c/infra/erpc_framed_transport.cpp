/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_framed_transport.h"

#include "erpc_config_internal.h"
#include ENDIANNESS_HEADER
#include "erpc_message_buffer.h"

#include <cstdio>

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

FramedTransport::FramedTransport(void)
: Transport()
, m_crcImpl(NULL)
#if !ERPC_THREADS_IS(NONE)
, m_sendLock()
, m_receiveLock()
#endif
{
}

FramedTransport::~FramedTransport(void) {}

void FramedTransport::setCrc16(Crc16 *crcImpl)
{
    erpc_assert(crcImpl);
    m_crcImpl = crcImpl;
}

erpc_status_t FramedTransport::receive(MessageBuffer *message)
{
    Header h;
    erpc_status_t retVal;
    uint16_t computedCrc;

    erpc_assert((m_crcImpl != NULL) && ("Uninitialized Crc16 object." != NULL));

    {
#if !ERPC_THREADS_IS(NONE)
        Mutex::Guard lock(m_receiveLock);
#endif

        // Receive header first.
        retVal = underlyingReceive((uint8_t *)&h, sizeof(h));

        if (retVal == kErpcStatus_Success)
        {
            ERPC_READ_AGNOSTIC_16(h.m_messageSize);
            ERPC_READ_AGNOSTIC_16(h.m_crc);

            // received size can't be zero.
            if (h.m_messageSize == 0U)
            {
                retVal = kErpcStatus_ReceiveFailed;
            }
        }

        if (retVal == kErpcStatus_Success)
        {
            // received size can't be larger then buffer length.
            if (h.m_messageSize > message->getLength())
            {
                retVal = kErpcStatus_ReceiveFailed;
            }
        }

        if (retVal == kErpcStatus_Success)
        {
            // Receive rest of the message now we know its size.
            retVal = underlyingReceive(message->get(), h.m_messageSize);
        }
    }

    if (retVal == kErpcStatus_Success)
    {
        // Verify CRC.
        computedCrc = m_crcImpl->computeCRC16(message->get(), h.m_messageSize);
        if (computedCrc == h.m_crc)
        {
            message->setUsed(h.m_messageSize);
        }
        else
        {
            retVal = kErpcStatus_CrcCheckFailed;
        }
    }

    return retVal;
}

erpc_status_t FramedTransport::send(MessageBuffer *message)
{
    erpc_status_t ret;
    uint16_t messageLength;
    Header h;

    erpc_assert((m_crcImpl != NULL) && ("Uninitialized Crc16 object." != NULL));

#if !ERPC_THREADS_IS(NONE)
    Mutex::Guard lock(m_sendLock);
#endif

    messageLength = message->getUsed();

    // Send header first.
    h.m_messageSize = messageLength;
    h.m_crc = m_crcImpl->computeCRC16(message->get(), messageLength);

    ERPC_WRITE_AGNOSTIC_16(h.m_messageSize);
    ERPC_WRITE_AGNOSTIC_16(h.m_crc);

    ret = underlyingSend((uint8_t *)&h, sizeof(h));
    if (ret == kErpcStatus_Success)
    {
        ret = underlyingSend(message->get(), messageLength);
    }

    return ret;
}
