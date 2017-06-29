/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
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
 * o Neither the name of the copyright holder nor the names of its
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

#ifndef _EMBEDDED_RPC__CODEC_H_
#define _EMBEDDED_RPC__CODEC_H_

#include "erpc_common.h"
#include "message_buffer.h"
#include "transport.h"
#include <cstring>
#include <stdint.h>

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
typedef enum _message_type {
    kInvocationMessage = 0,
    kOnewayMessage,
    kReplyMessage,
    kNotificationMessage
} message_type_t;

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
    Codec()
    : m_buffer()
    , m_cursor()
    {
    }

    /*!
     * @brief Codec destructor
     */
    virtual ~Codec() {}

    /*!
     * @brief Return message buffer used for read and write data.
     *
     * @return Pointer to used message buffer.
     */
    MessageBuffer *getBuffer() { return &m_buffer; }

    /*!
     * @brief Prototype for set message buffer used for read and write data.
     *
     * @param[in] buf Message buffer to set.
     */
    virtual void setBuffer(MessageBuffer &buf)
    {
        m_buffer = buf;
        m_cursor.set(&m_buffer);
    }

    /*! @brief Reset the codec to initial state. */
    virtual void reset() { m_cursor.set(&m_buffer); }

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
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t startWriteMessage(message_type_t type,
                                            uint32_t service,
                                            uint32_t request,
                                            uint32_t sequence) = 0;

    /*!
     * @brief Prototype for write end of message.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t endWriteMessage() = 0;

    /*!
     * @brief Prototype for write boolean value.
     *
     * @param[in] value Boolean typed value to write.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t write(bool value) = 0;

    /*!
     * @brief Prototype for write int8_t value.
     *
     * @param[in] value int8_t typed value to write.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t write(int8_t value) = 0;

    /*!
     * @brief Prototype for write int16_t value.
     *
     * @param[in] value int16_t typed value to write.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t write(int16_t value) = 0;

    /*!
     * @brief Prototype for write int32_t value.
     *
     * @param[in] value int32_t typed value to write.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t write(int32_t value) = 0;

    /*!
     * @brief Prototype for write int64_t value.
     *
     * @param[in] value int64_t typed value to write.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t write(int64_t value) = 0;

    /*!
     * @brief Prototype for write uint8_t value.
     *
     * @param[in] value uint8_t typed value to write.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t write(uint8_t value) = 0;

    /*!
     * @brief Prototype for write uint16_t value.
     *
     * @param[in] value uint16_t typed value to write.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t write(uint16_t value) = 0;

    /*!
     * @brief Prototype for write uint32_t value.
     *
     * @param[in] value uint32_t typed value to write.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t write(uint32_t value) = 0;

    /*!
     * @brief Prototype for write uint64_t value.
     *
     * @param[in] value uint64_t typed value to write.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t write(uint64_t value) = 0;

    /*!
     * @brief Prototype for write float value.
     *
     * @param[in] value float typed value to write.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t write(float value) = 0;

    /*!
     * @brief Prototype for write double value.
     *
     * @param[in] value double typed value to write.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t write(double value) = 0;

    /*!
     * @brief Prototype for write uintptr value.
     *
     * @param[in] value uintptr typed value to write.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t writePtr(uintptr_t value) = 0;

    /*!
     * @brief Prototype for write string value.
     *
     * @param[in] length of string.
     * @param[in] value string value to write.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t writeString(uint32_t length, const char *value) = 0;

    /*!
     * @brief Prototype for write binary value.
     *
     * @param[in] length of binary.
     * @param[in] value Binary value to write.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t writeBinary(uint32_t length, const uint8_t *value) = 0;

    /*!
     * @brief Prototype for start write list.
     *
     * @param[in] length Length of list.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t startWriteList(uint32_t length) = 0;

    /*!
     * @brief Prototype for end write list.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t endWriteList() = 0;

    /*!
     * @brief Prototype for start write structure.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t startWriteStruct() = 0;

    /*!
     * @brief Prototype for end write structure.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t endWriteStruct() = 0;

    /*!
     * @brief Prototype for start write union.
     *
     * @param[in] discriminator Discriminator of union.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t startWriteUnion(int32_t discriminator) = 0;

    /*!
     * @brief Prototype for end write union.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t endWriteUnion() = 0;

    /*!
     * @brief Writes a flag indicating whether the next value is null.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t writeNullFlag(bool isNull) = 0;
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
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t startReadMessage(message_type_t *type,
                                           uint32_t *service,
                                           uint32_t *request,
                                           uint32_t *sequence) = 0;

    /*!
     * @brief Prototype for read end of message.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t endReadMessage() = 0;

    /*!
     * @brief Prototype for read boolean value.
     *
     * @param[in] value Boolean typed value to read.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t read(bool *value) = 0;

    /*!
     * @brief Prototype for read int8_t value.
     *
     * @param[in] value int8_t typed value to read.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t read(int8_t *value) = 0;

    /*!
     * @brief Prototype for read int16_t value.
     *
     * @param[in] value int16_t typed value to read.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t read(int16_t *value) = 0;

    /*!
     * @brief Prototype for read int32_t value.
     *
     * @param[in] value int32_t typed value to read.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t read(int32_t *value) = 0;

    /*!
     * @brief Prototype for read int64_t value.
     *
     * @param[in] value int64_t typed value to read.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t read(int64_t *value) = 0;

    /*!
     * @brief Prototype for read uint8_t value.
     *
     * @param[in] value uint8_t typed value to read.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t read(uint8_t *value) = 0;

    /*!
     * @brief Prototype for read uint16_t value.
     *
     * @param[in] value uint16_t typed value to read.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t read(uint16_t *value) = 0;

    /*!
     * @brief Prototype for read uint32_t value.
     *
     * @param[in] value uint32_t typed value to read.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t read(uint32_t *value) = 0;

    /*!
     * @brief Prototype for read uint64_t value.
     *
     * @param[in] value uint64_t typed value to read.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t read(uint64_t *value) = 0;

    /*!
     * @brief Prototype for read float value.
     *
     * @param[in] value float typed value to read.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t read(float *value) = 0;

    /*!
     * @brief Prototype for read double value.
     *
     * @param[in] value double typed value to read.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t read(double *value) = 0;

    /*!
     * @brief Prototype for read uintptr value.
     *
     * @param[in] value uintptr typed value to read.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t readPtr(uintptr_t *value) = 0;

    /*!
     * @brief Prototype for read string value.
     *
     * @param[in] length of string.
     * @param[in] value String value to read.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t readString(uint32_t *length, char **value) = 0;

    /*!
     * @brief Prototype for read binary value.
     *
     * @param[in] length of binary.
     * @param[in] value Binary value to read.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t readBinary(uint32_t *length, uint8_t **value) = 0;

    /*!
     * @brief Prototype for start read list.
     *
     * @param[in] length Length of list.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t startReadList(uint32_t *length) = 0;

    /*!
     * @brief Prototype for end read list.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t endReadList() = 0;

    /*!
     * @brief Prototype for start read structure.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t startReadStruct() = 0;

    /*!
     * @brief Prototype for end read structure.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t endReadStruct() = 0;

    /*!
     * @brief Prototype for start read union.
     *
     * @param[in] discriminator Discriminator of union.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t startReadUnion(int32_t *discriminator) = 0;

    /*!
     * @brief Prototype for end read Union.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t endReadUnion() = 0;

    /*!
     * @brief Reads a flag indicating whether the next value is null.
     *
     * @return Based on implementation.
     */
    virtual erpc_status_t readNullFlag(bool *isNull) = 0;

protected:
    MessageBuffer m_buffer;         /*!< Message buffer object */
    MessageBuffer::Cursor m_cursor; /*!< Copy data to message buffers. */
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
    CodecFactory() {}

    /*!
     * @brief CodecFactory destructor
     */
    virtual ~CodecFactory() {}

    /*!
     * @brief Return created codec.
     *
     * @return Pointer to created codec.
     */
    virtual Codec *create() = 0;

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
