/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_framed_transport.hpp"

#include "erpc_config_internal.h"
#include ENDIANNESS_HEADER
#include "erpc_message_buffer.hpp"

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

uint8_t FramedTransport::reserveHeaderSize(void)
{
    return sizeof(FramedTransport::Header::m_crcHeader) + sizeof(FramedTransport::Header::m_messageSize) +
           sizeof(FramedTransport::Header::m_crcBody);
}

void FramedTransport::setCrc16(Crc16 *crcImpl)
{
    erpc_assert(crcImpl);
    m_crcImpl = crcImpl;
}

Crc16 *FramedTransport::getCrc16(void)
{
    return m_crcImpl;
}

erpc_status_t FramedTransport::receive(MessageBuffer *message)
{
    Header h = { 0, 0, 0 };
    erpc_status_t retVal;
    uint16_t computedCrc;
    uint8_t offset = 0;

    erpc_assert((m_crcImpl != NULL) && ("Uninitialized Crc16 object." != NULL));

    if (message->getLength() < reserveHeaderSize())
    {
        retVal = kErpcStatus_MemoryError;
    }
    else
    {
#if !ERPC_THREADS_IS(NONE)
        Mutex::Guard lock(m_receiveLock);
#endif

        // Receive header first.
        retVal = underlyingReceive(message, reserveHeaderSize(), 0);
        static_cast<void>(memcpy(&h.m_crcHeader, message->get(), sizeof(h.m_crcHeader)));
        offset = sizeof(h.m_crcHeader);
        static_cast<void>(memcpy(&h.m_messageSize, &message->get()[offset], sizeof(h.m_messageSize)));
        offset += sizeof(h.m_messageSize);
        static_cast<void>(memcpy(&h.m_crcBody, &message->get()[offset], sizeof(h.m_crcBody)));
        offset += sizeof(h.m_crcBody);

        if (retVal == kErpcStatus_Success)
        {
            ERPC_READ_AGNOSTIC_16(h.m_crcHeader);
            ERPC_READ_AGNOSTIC_16(h.m_messageSize);
            ERPC_READ_AGNOSTIC_16(h.m_crcBody);

            computedCrc =
                m_crcImpl->computeCRC16(reinterpret_cast<const uint8_t *>(&h.m_messageSize), sizeof(h.m_messageSize)) +
                m_crcImpl->computeCRC16(reinterpret_cast<const uint8_t *>(&h.m_crcBody), sizeof(h.m_crcBody));
            if (computedCrc != h.m_crcHeader)
            {
                retVal = kErpcStatus_CrcCheckFailed;
            }
        }

        if (retVal == kErpcStatus_Success)
        {
            // received size can't be larger then buffer length.
            if ((h.m_messageSize + reserveHeaderSize()) > message->getLength())
            {
                retVal = kErpcStatus_ReceiveFailed;
            }
        }

        if (retVal == kErpcStatus_Success)
        {
            // rpmsg tty can receive all data in one buffer, others need second call.
            if (message->getUsed() < (h.m_messageSize + reserveHeaderSize()))
            {
                // Receive rest of the message now we know its size.
                retVal = underlyingReceive(message, h.m_messageSize, offset);
            }
        }
    }

    if (retVal == kErpcStatus_Success)
    {
        // Verify CRC.
        computedCrc = m_crcImpl->computeCRC16(&message->get()[offset], h.m_messageSize);
        if (computedCrc != h.m_crcBody)
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
    uint8_t offset;

    erpc_assert((m_crcImpl != NULL) && ("Uninitialized Crc16 object." != NULL));

    messageLength = message->getUsed() - reserveHeaderSize();

    // Send header first.
    h.m_messageSize = messageLength;
    h.m_crcBody = m_crcImpl->computeCRC16(&message->get()[reserveHeaderSize()], messageLength);
    h.m_crcHeader =
        m_crcImpl->computeCRC16(reinterpret_cast<const uint8_t *>(&h.m_messageSize), sizeof(h.m_messageSize)) +
        m_crcImpl->computeCRC16(reinterpret_cast<const uint8_t *>(&h.m_crcBody), sizeof(h.m_crcBody));

    ERPC_WRITE_AGNOSTIC_16(h.m_crcHeader);
    ERPC_WRITE_AGNOSTIC_16(h.m_messageSize);
    ERPC_WRITE_AGNOSTIC_16(h.m_crcBody);

    static_cast<void>(memcpy(message->get(), reinterpret_cast<const uint8_t *>(&h.m_crcHeader), sizeof(h.m_crcHeader)));
    offset = sizeof(h.m_crcHeader);
    static_cast<void>(
        memcpy(&message->get()[offset], reinterpret_cast<const uint8_t *>(&h.m_messageSize), sizeof(h.m_messageSize)));
    offset += sizeof(h.m_messageSize);
    static_cast<void>(
        memcpy(&message->get()[offset], reinterpret_cast<const uint8_t *>(&h.m_crcBody), sizeof(h.m_crcBody)));

    ret = underlyingSend(message, message->getUsed(), 0);

    return ret;
}

erpc_status_t FramedTransport::underlyingSend(MessageBuffer *message, uint32_t size, uint32_t offset)
{
    erpc_status_t retVal = underlyingSend(&message->get()[offset], size);

    return retVal;
}

erpc_status_t FramedTransport::underlyingReceive(MessageBuffer *message, uint32_t size, uint32_t offset)
{
    erpc_status_t retVal = underlyingReceive(&message->get()[offset], size);

    if (retVal == kErpcStatus_Success)
    {
        message->setUsed(size + offset);
    }

    return retVal;
}
