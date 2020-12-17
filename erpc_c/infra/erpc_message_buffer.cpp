/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_message_buffer.h"
#include "erpc_config.h"

#include <cstring>

using namespace erpc;
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_status_t MessageBuffer::read(uint16_t offset, void *data, uint32_t length)
{
    if (offset + length > m_len)
    {
        return kErpcStatus_BufferOverrun;
    }

    if (length > 0)
    {
        memcpy(data, m_buf + offset, length);
    }

    return kErpcStatus_Success;
}

erpc_status_t MessageBuffer::write(uint16_t offset, const void *data, uint32_t length)
{
    if (offset + length > m_len)
    {
        return kErpcStatus_BufferOverrun;
    }

    if (length > 0)
    {
        memcpy(m_buf, data, length);
    }

    return kErpcStatus_Success;
}

erpc_status_t MessageBuffer::copy(const MessageBuffer *other)
{
    erpc_assert(m_len >= other->m_len);
    m_used = other->m_used;
    memcpy(m_buf, other->m_buf, m_used);

    return kErpcStatus_Success;
}

void MessageBuffer::swap(MessageBuffer *other)
{
    erpc_assert(other);
    MessageBuffer temp(*other);
    other->m_len = m_len;
    other->m_used = m_used;
    other->m_buf = m_buf;
    m_len = temp.m_len;
    m_used = temp.m_used;
    m_buf = temp.m_buf;
}

void MessageBuffer::Cursor::set(MessageBuffer *buffer)
{
    m_buffer = buffer;
    // RPMSG when nested calls are enabled can set NULL buffer.
    // erpc_assert(buffer->get() && "Data buffer wasn't set to MessageBuffer.");
    // receive function should return err if it couldn't set data buffer.
    m_pos = buffer->get();
    m_remaining = buffer->getLength();
}

erpc_status_t MessageBuffer::Cursor::read(void *data, uint32_t length)
{
    erpc_assert(m_pos && "Data buffer wasn't set to MessageBuffer.");
    if (m_remaining < length)
    {
        return kErpcStatus_BufferOverrun;
    }

    memcpy(data, m_pos, length);
    m_pos += length;
    m_remaining -= length;

    return kErpcStatus_Success;
}

erpc_status_t MessageBuffer::Cursor::write(const void *data, uint32_t length)
{
    erpc_assert(m_pos && "Data buffer wasn't set to MessageBuffer.");
    if (length > m_remaining)
    {
        return kErpcStatus_BufferOverrun;
    }

    memcpy(m_pos, data, length);
    m_pos += length;
    m_remaining -= length;
    m_buffer->setUsed(m_buffer->getUsed() + length);

    return kErpcStatus_Success;
}

erpc_status_t MessageBufferFactory::prepareServerBufferForSend(MessageBuffer *message)
{
    message->setUsed(0);
    return kErpcStatus_Success;
}
