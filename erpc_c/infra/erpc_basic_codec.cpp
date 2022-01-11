/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_basic_codec.h"

#include "erpc_manually_constructed.h"

#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
#include <new>
#endif
#include <cassert>

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

const uint8_t BasicCodec::kBasicCodecVersion = 1;

void BasicCodec::startWriteMessage(message_type_t type, uint32_t service, uint32_t request, uint32_t sequence)
{
    uint32_t header = (kBasicCodecVersion << 24) | ((service & 0xff) << 16) | ((request & 0xff) << 8) | (type & 0xff);

    write(header);

    write(sequence);
}

void BasicCodec::writeData(const void *value, uint32_t length)
{
    if (!m_status)
    {
        if (value != NULL)
        {
            m_status = m_cursor.write(value, length);
        }
        else
        {
            m_status = kErpcStatus_MemoryError;
        }
    }
}

void BasicCodec::write(bool value)
{
    // Make sure the bool is a single byte.
    uint8_t v = value;

    writeData(&v, sizeof(v));
}

void BasicCodec::write(int8_t value)
{
    writeData(&value, sizeof(value));
}

void BasicCodec::write(int16_t value)
{
    writeData(&value, sizeof(value));
}

void BasicCodec::write(int32_t value)
{
    writeData(&value, sizeof(value));
}

void BasicCodec::write(int64_t value)
{
    writeData(&value, sizeof(value));
}

void BasicCodec::write(uint8_t value)
{
    writeData(&value, sizeof(value));
}

void BasicCodec::write(uint16_t value)
{
    writeData(&value, sizeof(value));
}

void BasicCodec::write(uint32_t value)
{
    writeData(&value, sizeof(value));
}

void BasicCodec::write(uint64_t value)
{
    writeData(&value, sizeof(value));
}

void BasicCodec::write(float value)
{
    writeData(&value, sizeof(value));
}

void BasicCodec::write(double value)
{
    writeData(&value, sizeof(value));
}

void BasicCodec::writePtr(uintptr_t value)
{
    uint8_t ptrSize = sizeof(value);

    write(ptrSize);

    writeData(&value, ptrSize);
}

void BasicCodec::writeString(uint32_t length, const char *value)
{
    // Just treat the string as binary.
    writeBinary(length, reinterpret_cast<const uint8_t *>(value));
}

void BasicCodec::writeBinary(uint32_t length, const uint8_t *value)
{
    // Write the blob length as a u32.
    write(length);

    writeData(value, length);
}

void BasicCodec::startWriteList(uint32_t length)
{
    // Write the list length as a u32.
    write(length);
}

void BasicCodec::startWriteUnion(int32_t discriminator)
{
    // Write the union discriminator as a u32.
    write(discriminator);
}

void BasicCodec::writeNullFlag(bool isNull)
{
    write(static_cast<uint8_t>(isNull ? kIsNull : kNotNull));
}

void BasicCodec::writeCallback(arrayOfFunPtr callbacks, uint8_t callbacksCount, funPtr callback)
{
    uint8_t i;

    assert(callbacksCount > 1U);

    // callbacks = callbacks table
    for (i = 0; i < callbacksCount; i++)
    {
        if (callbacks[i] == callback)
        {
            write(i);
            break;
        }
        if ((i + 1U) == callbacksCount)
        {
            updateStatus(kErpcStatus_UnknownCallback);
        }
    }
}

void BasicCodec::writeCallback(funPtr callback1, funPtr callback2)
{
    // callbacks = callback directly
    // When declared only one callback function no serialization is needed.
    if (callback1 != callback2)
    {
        updateStatus(kErpcStatus_UnknownCallback);
    }
}

void BasicCodec::startReadMessage(message_type_t *type, uint32_t *service, uint32_t *request, uint32_t *sequence)
{
    uint32_t header;

    read(&header);

    if (((header >> 24) & 0xffU) != kBasicCodecVersion)
    {
        updateStatus(kErpcStatus_InvalidMessageVersion);
    }

    if (!m_status)
    {
        *service = ((header >> 16) & 0xffU);
        *request = ((header >> 8) & 0xffU);
        *type = static_cast<message_type_t>(header & 0xffU);

        read(sequence);
    }
}

void BasicCodec::readData(void *value, uint32_t length)
{
    if (!m_status)
    {
        if (value != NULL)
        {
            m_status = m_cursor.read(value, length);
        }
        else
        {
            m_status = kErpcStatus_MemoryError;
        }
    }
}

void BasicCodec::read(bool *value)
{
    uint8_t v = 0;

    readData(&v, sizeof(v));
    if (!m_status)
    {
        *value = v;
    }
}

void BasicCodec::read(int8_t *value)
{
    readData(value, sizeof(*value));
}

void BasicCodec::read(int16_t *value)
{
    readData(value, sizeof(*value));
}

void BasicCodec::read(int32_t *value)
{
    readData(value, sizeof(*value));
}

void BasicCodec::read(int64_t *value)
{
    readData(value, sizeof(*value));
}

void BasicCodec::read(uint8_t *value)
{
    readData(value, sizeof(*value));
}

void BasicCodec::read(uint16_t *value)
{
    readData(value, sizeof(*value));
}

void BasicCodec::read(uint32_t *value)
{
    readData(value, sizeof(*value));
}

void BasicCodec::read(uint64_t *value)
{
    readData(value, sizeof(*value));
}

void BasicCodec::read(float *value)
{
    readData(value, sizeof(*value));
}

void BasicCodec::read(double *value)
{
    readData(value, sizeof(*value));
}

void BasicCodec::readPtr(uintptr_t *value)
{
    uint8_t ptrSize;

    read(&ptrSize);

    if (ptrSize > sizeof(*value))
    {
        updateStatus(kErpcStatus_BadAddressScale);
    }

    readData(value, ptrSize);
}

void BasicCodec::readString(uint32_t *length, char **value)
{
    readBinary(length, reinterpret_cast<uint8_t **>(value));
}

void BasicCodec::readBinary(uint32_t *length, uint8_t **value)
{
    // Read length first as u32.
    read(length);

    if (!m_status)
    {
        if (m_cursor.getRemaining() >= *length)
        {
            // Return current pointer into buffer.
            *value = m_cursor.get();

            // Skip over data.
            m_cursor += *length;
        }
        else
        {
            *length = 0;
            m_status = kErpcStatus_BufferOverrun;
        }
    }
    else
    {
        *length = 0;
    }
}

void BasicCodec::startReadList(uint32_t *length)
{
    // Read list length as u32.
    read(length);

    if (!isStatusOk())
    {
        *length = 0;
    }
}

void BasicCodec::startReadUnion(int32_t *discriminator)
{
    // Read union discriminator as u32.
    read(discriminator);
}

void BasicCodec::readNullFlag(bool *isNull)
{
    uint8_t flag;

    read(&flag);
    if (isStatusOk())
    {
        *isNull = (flag == (uint8_t)kIsNull);
    }
}

void BasicCodec::readCallback(arrayOfFunPtr callbacks, uint8_t callbacksCount, funPtr *callback)
{
    uint8_t _tmp_local;

    assert(callbacksCount > 1U);

    // callbacks = callbacks table
    read(&_tmp_local);
    if (isStatusOk())
    {
        if (_tmp_local < callbacksCount)
        {
            *callback = callbacks[_tmp_local];
        }
        else
        {
            m_status = kErpcStatus_UnknownCallback;
        }
    }
}

void BasicCodec::readCallback(funPtr callbacks1, funPtr *callback2)
{
    // callbacks = callback directly
    *callback2 = callbacks1;
}

ERPC_MANUALLY_CONSTRUCTED_ARRAY_STATIC(BasicCodec, s_basicCodecManual, ERPC_CODEC_COUNT);

Codec *BasicCodecFactory::create()
{
    ERPC_CREATE_NEW_OBJECT(BasicCodec, s_basicCodecManual, ERPC_CODEC_COUNT)
}

void BasicCodecFactory::dispose(Codec *codec)
{
    ERPC_DESTROY_OBJECT(codec, s_basicCodecManual, ERPC_CODEC_COUNT)
}
