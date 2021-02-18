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

#include "erpc_message_buffer.h"

#include <cassert>
#include <cstdio>

#include "stdlib.h"
#include <iostream>
#include <sstream>
#include <iomanip>

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
    assert(crcImpl);
    m_crcImpl = crcImpl;
}

erpc_status_t FramedTransport::receive(MessageBuffer *message)
{
    assert(m_crcImpl && "Uninitialized Crc16 object.");
    Header h;
    uint8_t header[14] = {0};
    uint8_t temp[5] = {0};

    {
#if !ERPC_THREADS_IS(NONE)
        Mutex::Guard lock(m_receiveLock);
#endif

        // Receive header first.
        erpc_status_t ret = underlyingReceive(header, sizeof(header));
        if (ret != kErpcStatus_Success) {
            return ret;
        }

        // frame error
        if((header[0] != ':') || (header[13] != '\n')) {
            return kErpcStatus_ReceiveFailed;
        }

        // get message size
        memcpy(temp, &header[1], 4);
        h.m_messageSize = (uint16_t)strtol((const char*)temp, NULL, 16);
        // received size can't be larger then buffer length.
        if (h.m_messageSize > message->getLength()) {
            return kErpcStatus_ReceiveFailed;
        }

        // get message CRC
        memcpy(temp, &header[5], 4);
        h.m_crc = (uint16_t)strtol((const char*)temp, NULL, 16);

        // get and compute header CRC
        memcpy(temp, &header[9], 4);
        uint16_t headerReceivedCRC = (uint16_t)strtol((const char*)temp, NULL, 16);
        uint16_t headerComputedCRC = m_crcImpl->computeCRC16((uint8_t *)&h, sizeof(h));

        if(headerComputedCRC != headerReceivedCRC) {
            return kErpcStatus_ReceiveFailed;
        }

        // Receive rest of the message now we know its size.
        ret = underlyingReceive(message->get(), h.m_messageSize);
        if (ret != kErpcStatus_Success)
        {
            return ret;
        }
    }

    // Verify CRC.
    uint16_t computedCrc = m_crcImpl->computeCRC16(message->get(), h.m_messageSize);
    if (computedCrc != h.m_crc)
    {
        return kErpcStatus_CrcCheckFailed;
    }

    message->setUsed(h.m_messageSize);
    return kErpcStatus_Success;
}

erpc_status_t FramedTransport::send(MessageBuffer *message)
{
    assert(m_crcImpl && "Uninitialized Crc16 object.");
    Header h;

#if !ERPC_THREADS_IS(NONE)
    Mutex::Guard lock(m_sendLock);
#endif

    h.m_messageSize = message->getUsed();
    h.m_crc = m_crcImpl->computeCRC16(message->get(), h.m_messageSize);
    uint16_t headerCRC = m_crcImpl->computeCRC16((const uint8_t *)&h, 4);

    std::stringstream ss;
    ss << ':' << std::hex
        << std::setfill('0') << std::setw(4) << h.m_messageSize
        << std::setfill('0') << std::setw(4) << h.m_crc
        << std::setfill('0') << std::setw(4) << headerCRC
    << "\n";

    erpc_status_t ret = underlyingSend((uint8_t *)ss.str().c_str(), ss.str().size());
    if (ret != kErpcStatus_Success) {
        return ret;
    }

    // Send the rest of the message.
    return underlyingSend(message->get(), h.m_messageSize);
}
