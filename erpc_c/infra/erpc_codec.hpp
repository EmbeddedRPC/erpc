/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__CODEC_H_
#define _EMBEDDED_RPC__CODEC_H_

#include "erpc_common.h"
#include "erpc_message_buffer.hpp"
#include "erpc_transport.hpp"

#include <cstdint>
#include <cstring>

/*!
 * @addtogroup infra_codec
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc {
/*!
 * @brief Types of messages that can be encoded.
 */
enum class message_type_t
{
    kInvocationMessage = 0,
    kOnewayMessage,
    kReplyMessage,
    kNotificationMessage
};

typedef void *funPtr;          // Pointer to functions
typedef funPtr *arrayOfFunPtr; // Pointer to array of functions

/*!
 * @brief Abstract serialization encoder/decoder interface.
 *
 * Codecs write to or read from a MessageBuffer.
 *
 * @ingroup infra_codec
 */
class Codec
{
public:
    /*!
     * @brief Constructor.
     *
     * This function initializes object attributes.
     */
    Codec(void) : m_cursor(), m_status(kErpcStatus_Success) {}

    /*!
     * @brief Codec destructor
     */
    virtual ~Codec(void) {}

    /*!
     * @brief Return message buffer used for read and write data.
     *
     * @return Pointer to used message buffer.
     */
    MessageBuffer getBuffer(void) { return m_cursor.getBuffer(); }

    MessageBuffer &getBufferRef(void) { return m_cursor.getBufferRef(); }

    /*!
     * @brief Prototype for set message buffer used for read and write data.
     *
     * @param[in] buf Message buffer to set.
     * @param[in] skip How many bytes to skip from reading.
     */
    virtual void setBuffer(MessageBuffer &buf, uint8_t skip = 0)
    {
        m_cursor.setBuffer(buf, skip);
        m_status = kErpcStatus_Success;
    }

    /*!
     * @brief Reset the codec to initial state.
     *
     * @param[in] skip How many bytes to skip from reading.
     */
    virtual void reset(uint8_t skip = 0)
    {
        MessageBuffer buffer = m_cursor.getBuffer();
        m_cursor.setBuffer(buffer, skip);
        m_status = kErpcStatus_Success;
    }

    /*!
     * @brief Return current status of eRPC message processing.
     *
     * @return Current status of eRPC message processing.
     */
    erpc_status_t getStatus(void) { return m_status; }

    /*!
     * @brief Return bool value representing current status.
     *
     * @retval True Current status value is kErpcStatus_Success.
     * @retval False Current status is other than kErpcStatus_Success.
     */
    bool isStatusOk(void) { return (m_status == kErpcStatus_Success); }

    /*!
     * @brief Set current status of eRPC message processing to given value.
     *
     * @param[in] status New current value.
     */
    void updateStatus(erpc_status_t status)
    {
        if (isStatusOk())
        {
            m_status = status;
        }
    }

    //! @name Encoding
    //@{
    /*!
     * @brief Prototype for write header of message.
     *
     * @param[in] type Type of message.
     * @param[in] service Which interface is requested.
     * @param[in] request Which function need be called.
     * @param[in] sequence Send sequence number to be sure that
     *                    received message is reply for current request.
     */
    virtual void startWriteMessage(message_type_t type, uint32_t service, uint32_t request, uint32_t sequence) = 0;

    /*!
     * @brief Prototype for write boolean value.
     *
     * @param[in] value Boolean typed value to write.
     */
    virtual void write(bool value) = 0;

    /*!
     * @brief Prototype for write int8_t value.
     *
     * @param[in] value int8_t typed value to write.
     */
    virtual void write(int8_t value) = 0;

    /*!
     * @brief Prototype for write int16_t value.
     *
     * @param[in] value int16_t typed value to write.
     */
    virtual void write(int16_t value) = 0;

    /*!
     * @brief Prototype for write int32_t value.
     *
     * @param[in] value int32_t typed value to write.
     */
    virtual void write(int32_t value) = 0;

    /*!
     * @brief Prototype for write int64_t value.
     *
     * @param[in] value int64_t typed value to write.
     */
    virtual void write(int64_t value) = 0;

    /*!
     * @brief Prototype for write uint8_t value.
     *
     * @param[in] value uint8_t typed value to write.
     */
    virtual void write(uint8_t value) = 0;

    /*!
     * @brief Prototype for write uint16_t value.
     *
     * @param[in] value uint16_t typed value to write.
     */
    virtual void write(uint16_t value) = 0;

    /*!
     * @brief Prototype for write uint32_t value.
     *
     * @param[in] value uint32_t typed value to write.
     */
    virtual void write(uint32_t value) = 0;

    /*!
     * @brief Prototype for write uint64_t value.
     *
     * @param[in] value uint64_t typed value to write.
     */
    virtual void write(uint64_t value) = 0;

    /*!
     * @brief Prototype for write float value.
     *
     * @param[in] value float typed value to write.
     */
    virtual void write(float value) = 0;

    /*!
     * @brief Prototype for write double value.
     *
     * @param[in] value double typed value to write.
     */
    virtual void write(double value) = 0;

    /*!
     * @brief Prototype for write uintptr value.
     *
     * @param[in] value uintptr typed value to write.
     */
    virtual void writePtr(uintptr_t value) = 0;

    /*!
     * @brief Prototype for write string value.
     *
     * @param[in] length of string.
     * @param[in] value string value to write.
     */
    virtual void writeString(uint32_t length, const char *value) = 0;

    /*!
     * @brief Prototype for write binary value.
     *
     * @param[in] length of binary.
     * @param[in] value Binary value to write.
     */
    virtual void writeBinary(uint32_t length, const uint8_t *value) = 0;

    /*!
     * @brief Prototype for start write list.
     *
     * @param[in] length Length of list.
     */
    virtual void startWriteList(uint32_t length) = 0;

    /*!
     * @brief Prototype for start write union.
     *
     * @param[in] discriminator Discriminator of union.
     */
    virtual void startWriteUnion(int32_t discriminator) = 0;

    /*!
     * @brief Writes a flag indicating whether the next value is null.
     *
     * @param[in] isNull Null flag to send.
     */
    virtual void writeNullFlag(bool isNull) = 0;
    //@}

    //! @name Decoding
    //@{
    /*!
     * @brief Prototype for read header of message.
     *
     * @param[in] type Type of message.
     * @param[in] service Which interface was used.
     * @param[in] request Which function was called.
     * @param[in] sequence Returned sequence number to be sure that
     *                     received message is reply for current request.
     */
    virtual void startReadMessage(message_type_t &type, uint32_t &service, uint32_t &request, uint32_t &sequence) = 0;

    /*!
     * @brief Prototype for read boolean value.
     *
     * @param[in] value Boolean typed value to read.
     */
    virtual void read(bool &value) = 0;

    /*!
     * @brief Prototype for read int8_t value.
     *
     * @param[in] value int8_t typed value to read.
     */
    virtual void read(int8_t &value) = 0;

    /*!
     * @brief Prototype for read int16_t value.
     *
     * @param[in] value int16_t typed value to read.
     */
    virtual void read(int16_t &value) = 0;

    /*!
     * @brief Prototype for read int32_t value.
     *
     * @param[in] value int32_t typed value to read.
     */
    virtual void read(int32_t &value) = 0;

    /*!
     * @brief Prototype for read int64_t value.
     *
     * @param[in] value int64_t typed value to read.
     */
    virtual void read(int64_t &value) = 0;

    /*!
     * @brief Prototype for read uint8_t value.
     *
     * @param[in] value uint8_t typed value to read.
     */
    virtual void read(uint8_t &value) = 0;

    /*!
     * @brief Prototype for read uint16_t value.
     *
     * @param[in] value uint16_t typed value to read.
     */
    virtual void read(uint16_t &value) = 0;

    /*!
     * @brief Prototype for read uint32_t value.
     *
     * @param[in] value uint32_t typed value to read.
     */
    virtual void read(uint32_t &value) = 0;

    /*!
     * @brief Prototype for read uint64_t value.
     *
     * @param[in] value uint64_t typed value to read.
     */
    virtual void read(uint64_t &value) = 0;

    /*!
     * @brief Prototype for read float value.
     *
     * @param[in] value float typed value to read.
     */
    virtual void read(float &value) = 0;

    /*!
     * @brief Prototype for read double value.
     *
     * @param[in] value double typed value to read.
     */
    virtual void read(double &value) = 0;

    /*!
     * @brief Prototype for read uintptr value.
     *
     * @param[in] value uintptr typed value to read.
     */
    virtual void readPtr(uintptr_t &value) = 0;

    /*!
     * @brief Prototype for read string value.
     *
     * @param[in] length of string.
     * @param[in] value String value to read.
     */
    virtual void readString(uint32_t &length, char **value) = 0;

    /*!
     * @brief Prototype for read binary value.
     *
     * @param[out] length of binary. 0 can be valid value or in case of error.
     * @param[out] value Binary value to read. Null in case of error.
     */
    virtual void readBinary(uint32_t &length, uint8_t **value) = 0;

    /*!
     * @brief Prototype for start read list.
     *
     * @param[in] length Length of list.
     */
    virtual void startReadList(uint32_t &length) = 0;

    /*!
     * @brief Prototype for start read union.
     *
     * @param[in] discriminator Discriminator of union.
     */
    virtual void startReadUnion(int32_t &discriminator) = 0;

    /*!
     * @brief Reads a flag indicating whether the next value is null.
     *
     * @param[in] isNull Null flag to read.
     */
    virtual void readNullFlag(bool &isNull) = 0;

protected:
    Cursor m_cursor;        /*!< Copy data to message buffers. */
    erpc_status_t m_status; /*!< Status of serialized data. */
};

/*!
 * @brief Abstract interface for codec factory.
 *
 * @ingroup infra_codec
 */
class CodecFactory
{
public:
    /*!
     * @brief Constructor.
     */
    CodecFactory(void) {}

    /*!
     * @brief CodecFactory destructor
     */
    virtual ~CodecFactory(void) {}

    /*!
     * @brief Return created codec.
     *
     * @return Pointer to created codec.
     */
    virtual Codec *create(void) = 0;

    /*!
     * @brief Dispose codec.
     *
     * @param[in] codec Codec to dispose.
     */
    virtual void dispose(Codec *codec) = 0;
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__CODEC_H_
