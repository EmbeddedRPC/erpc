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

#include "basic_codec.h"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

status_t BasicCodec::startWriteMessage(message_type_t type, uint32_t service, uint32_t request, uint32_t sequence)
{
    uint32_t header = (kBasicCodecVersion << 24) | ((service & 0xff) << 16) | ((request & 0xff) << 8) | (type & 0xff);
    status_t err = write(header);
    if (err)
    {
        return err;
    }

    err = write(sequence); /* TODO: just return write(sequence);? same is used in startReadMessage */
    if (err)
    {
        return err;
    }

    return kErpcStatus_Success;
}

status_t BasicCodec::endWriteMessage()
{
    return kErpcStatus_Success;
}

status_t BasicCodec::write(bool value)
{
    // Make sure the bool is a single byte.
    uint8_t v = value;
    return m_cursor.write(&v, sizeof(v));
}

status_t BasicCodec::write(int8_t value)
{
    return m_cursor.write(&value, sizeof(value));
}

status_t BasicCodec::write(int16_t value)
{
    return m_cursor.write(&value, sizeof(value));
}

status_t BasicCodec::write(int32_t value)
{
    return m_cursor.write(&value, sizeof(value));
}

status_t BasicCodec::write(int64_t value)
{
    return m_cursor.write(&value, sizeof(value));
}

status_t BasicCodec::write(uint8_t value)
{
    return m_cursor.write(&value, sizeof(value));
}

status_t BasicCodec::write(uint16_t value)
{
    return m_cursor.write(&value, sizeof(value));
}

status_t BasicCodec::write(uint32_t value)
{
    return m_cursor.write(&value, sizeof(value));
}

status_t BasicCodec::write(uint64_t value)
{
    return m_cursor.write(&value, sizeof(value));
}

status_t BasicCodec::write(float value)
{
    return m_cursor.write(&value, sizeof(value));
}

status_t BasicCodec::write(double value)
{
    return m_cursor.write(&value, sizeof(value));
}

status_t BasicCodec::writeString(uint32_t length, const char *value)
{
    // Just treat the string as binary.
    return writeBinary(length, reinterpret_cast<const uint8_t *>(value));
}

status_t BasicCodec::writeBinary(uint32_t length, const uint8_t *value)
{
    // Write the blob length as a u32.
    status_t err = write(length);
    if (err)
    {
        return err;
    }

    return m_cursor.write(value, length);
}

status_t BasicCodec::startWriteList(uint32_t length)
{
    // Write the list length as a u32.
    return write(length);
}

status_t BasicCodec::endWriteList()
{
    return kErpcStatus_Success;
}

status_t BasicCodec::startWriteStruct()
{
    return kErpcStatus_Success;
}

status_t BasicCodec::endWriteStruct()
{
    return kErpcStatus_Success;
}

status_t BasicCodec::writeNullFlag(bool isNull)
{
    return write(static_cast<uint8_t>(isNull ? kIsNull : kNotNull));
}

status_t BasicCodec::startReadMessage(message_type_t *type, uint32_t *service, uint32_t *request, uint32_t *sequence)
{
    uint32_t header;
    status_t err = read(&header);
    if (err)
    {
        return err;
    }

    if (((header >> 24) & 0xff) != kBasicCodecVersion)
    {
        return kErpcStatus_InvalidMessageVersion;
    }

    *service = ((header >> 16) & 0xff);
    *request = ((header >> 8) & 0xff);
    *type = static_cast<message_type_t>(header & 0xff);

    return read(sequence);
}

status_t BasicCodec::endReadMessage()
{
    return kErpcStatus_Success;
}

status_t BasicCodec::read(bool *value)
{
    uint8_t v = 0;
    status_t err = m_cursor.read(&v, sizeof(v));
    *value = v;
    return err;
}

status_t BasicCodec::read(int8_t *value)
{
    return m_cursor.read(value, sizeof(*value));
}

status_t BasicCodec::read(int16_t *value)
{
    return m_cursor.read(value, sizeof(*value));
}

status_t BasicCodec::read(int32_t *value)
{
    return m_cursor.read(value, sizeof(*value));
}

status_t BasicCodec::read(int64_t *value)
{
    return m_cursor.read(value, sizeof(*value));
}

status_t BasicCodec::read(uint8_t *value)
{
    return m_cursor.read(value, sizeof(*value));
}

status_t BasicCodec::read(uint16_t *value)
{
    return m_cursor.read(value, sizeof(*value));
}

status_t BasicCodec::read(uint32_t *value)
{
    return m_cursor.read(value, sizeof(*value));
}

status_t BasicCodec::read(uint64_t *value)
{
    return m_cursor.read(value, sizeof(*value));
}

status_t BasicCodec::read(float *value)
{
    return m_cursor.read(value, sizeof(*value));
}

status_t BasicCodec::read(double *value)
{
    return m_cursor.read(value, sizeof(*value));
}

status_t BasicCodec::readString(uint32_t *length, char **value)
{
    return readBinary(length, reinterpret_cast<uint8_t **>(value));
}

status_t BasicCodec::readBinary(uint32_t *length, uint8_t **value)
{
    // Read length first as u32.
    status_t err = read(length);
    if (err)
    {
        return err;
    }

    // Return current pointer into buffer.
    *value = m_cursor.get();

    // Skip over data.
    m_cursor += *length;

    return kErpcStatus_Success;
}

status_t BasicCodec::startReadList(uint32_t *length)
{
    // Read list length as u32.
    return read(length);
}

status_t BasicCodec::endReadList()
{
    return kErpcStatus_Success;
}

status_t BasicCodec::startReadStruct()
{
    return kErpcStatus_Success;
}

status_t BasicCodec::endReadStruct()
{
    return kErpcStatus_Success;
}

status_t BasicCodec::readNullFlag(bool *isNull)
{
    uint8_t flag;
    status_t status = read(&flag);
    if (!status)
    {
        return status;
    }
    *isNull = (flag == kIsNull);
    return kErpcStatus_Success;
}
