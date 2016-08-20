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

#include <cstring>
#include <cassert>
#include "message_buffer.h"

using namespace erpc;
#if !(__embedded_cplusplus)
using namespace std;
#endif

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

status_t MessageBuffer::read(uint16_t offset, void *data, uint32_t length)
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

status_t MessageBuffer::write(uint16_t offset, const void *data, uint32_t length)
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

status_t MessageBuffer::copy(const MessageBuffer *other)
{
    assert(m_len >= other->m_len);
    m_used = other->m_used;
    memcpy(m_buf, other->m_buf, m_used);

    return kErpcStatus_Success;
}

void MessageBuffer::swap(MessageBuffer *other)
{
    assert(other);
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
    m_pos = buffer->get();
    m_remaining = buffer->getLength();
}

status_t MessageBuffer::Cursor::read(void *data, uint32_t length)
{
    if (m_remaining < length)
    {
        return kErpcStatus_BufferOverrun;
    }

    memcpy(data, m_pos, length);
    m_pos += length;
    m_remaining -= length;

    return kErpcStatus_Success;
}

status_t MessageBuffer::Cursor::write(const void *data, uint32_t length)
{
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
