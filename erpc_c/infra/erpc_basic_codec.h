/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__BASIC_SERIALIZATION_H_
#define _EMBEDDED_RPC__BASIC_SERIALIZATION_H_

#include "erpc_codec.h"
#include <new>

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
 * @brief Values of the uint8 flag prefixing nullable values.
 */
enum _null_flag
{
    kNotNull = 0,
    kIsNull
};

/*!
 * @brief Simple binary serialization format.
 *
 * @ingroup infra_codec
 */
class BasicCodec : public Codec
{
public:
    static const uint32_t kBasicCodecVersion; /*!< Codec version. */

    BasicCodec(void)
    : Codec()
    {
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
     *                    received message is reply for current request. or write function.
     */
    virtual void startWriteMessage(message_type_t type, uint32_t service, uint32_t request, uint32_t sequence);

    /*!
     * @brief Prototype for write data stream.
     *
     * @param[in] value Pointer to data stream.
     * @param[in] length Size of data stream in bytes.
     */
    virtual void writeData(const void *value, uint32_t length);

    /*!
     * @brief Prototype for write boolean value.
     *
     * @param[in] value Boolean typed value to write.
     */
    virtual void write(bool value);

    /*!
     * @brief Prototype for write int8_t value.
     *
     * @param[in] value int8_t typed value to write.
     */
    virtual void write(int8_t value);

    /*!
     * @brief Prototype for write int16_t value.
     *
     * @param[in] value int16_t typed value to write.
     */
    virtual void write(int16_t value);

    /*!
     * @brief Prototype for write int32_t value.
     *
     * @param[in] value int32_t typed value to write.
     */
    virtual void write(int32_t value);

    /*!
     * @brief Prototype for write int64_t value.
     *
     * @param[in] value int64_t typed value to write.
     */
    virtual void write(int64_t value);

    /*!
     * @brief Prototype for write uint8_t value.
     *
     * @param[in] value uint8_t typed value to write.
     */
    virtual void write(uint8_t value);

    /*!
     * @brief Prototype for write uint16_t value.
     *
     * @param[in] value uint16_t typed value to write.
     */
    virtual void write(uint16_t value);

    /*!
     * @brief Prototype for write uint32_t value.
     *
     * @param[in] value uint32_t typed value to write.
     */
    virtual void write(uint32_t value);

    /*!
     * @brief Prototype for write uint64_t value.
     *
     * @param[in] value uint64_t typed value to write.
     */
    virtual void write(uint64_t value);

    /*!
     * @brief Prototype for write float value.
     *
     * @param[in] value float typed value to write.
     */
    virtual void write(float value);

    /*!
     * @brief Prototype for write double value.
     *
     * @param[in] value double typed value to write.
     */
    virtual void write(double value);

    /*!
     * @brief Prototype for write uintptr value.
     *
     * @param[in] value uintptr typed value to write.
     */
    virtual void writePtr(uintptr_t value);

    /*!
     * @brief Prototype for write string value.
     *
     * @param[in] length of string.
     * @param[in] value string value to write.
     */
    virtual void writeString(uint32_t length, const char *value);

    /*!
     * @brief Prototype for write binary value.
     *
     * @param[in] length of binary.
     * @param[in] value Binary value to write.
     */
    virtual void writeBinary(uint32_t length, const uint8_t *value);

    /*!
     * @brief Prototype for start write list.
     *
     * @param[in] length Length of list.
     */
    virtual void startWriteList(uint32_t length);

    /*!
     * @brief Prototype for start write union.
     *
     * @param[in] discriminator Discriminator of union.
     */
    virtual void startWriteUnion(int32_t discriminator);

    /*!
     * @brief Writes a flag indicating whether the next value is null.
     *
     * @param[in] isNull Null flag to send.
     */
    virtual void writeNullFlag(bool isNull);

    /*!
     * @brief Writes an order ID of callback function.
     *
     * @param[in] callbacks Pointer to array of callbacks.
     * @param[in] callbacksCount Size of array of callbacks.
     * @param[in] callback Callback which ID should be serialized.
     */
    virtual void writeCallback(arrayOfFunPtr callbacks, uint8_t callbacksCount, funPtr callback);

    /*!
     * @brief Writes an order ID of callback function.
     *
     * @param[in] callback1 Pointer to existing callback.
     * @param[out] callback2 Callback which ID should be serialized.
     */
    virtual void writeCallback(funPtr callback1, funPtr callback2);
    //@}

    //! @name Decoding
    //@{
    /*!
     * @brief Prototype for read header of message.
     *
     * @param[out] type Type of message.
     * @param[out] service Which interface was used.
     * @param[out] request Which function was called.
     * @param[out] sequence Returned sequence number to be sure that
     *                     received message is reply for current request.
     */
    virtual void startReadMessage(message_type_t *type, uint32_t *service, uint32_t *request, uint32_t *sequence);

    /*!
     * @brief Prototype for read data stream.
     *
     * @param[in] value Pointer to data stream to be read.
     * @param[in] length Size of data stream in bytes to be read.
     */
    virtual void readData(void *value, uint32_t length);

    /*!
     * @brief Prototype for read boolean value.
     *
     * @param[out] value Boolean typed value to read.
     */
    virtual void read(bool *value);

    /*!
     * @brief Prototype for read int8_t value.
     *
     * @param[out] value int8_t typed value to read.
     */
    virtual void read(int8_t *value);

    /*!
     * @brief Prototype for read int16_t value.
     *
     * @param[out] value int16_t typed value to read.
     */
    virtual void read(int16_t *value);

    /*!
     * @brief Prototype for read int32_t value.
     *
     * @param[out] value int32_t typed value to read.
     */
    virtual void read(int32_t *value);

    /*!
     * @brief Prototype for read int64_t value.
     *
     * @param[out] value int64_t typed value to read.
     */
    virtual void read(int64_t *value);

    /*!
     * @brief Prototype for read uint8_t value.
     *
     * @param[out] value uint8_t typed value to read.
     */
    virtual void read(uint8_t *value);

    /*!
     * @brief Prototype for read uint16_t value.
     *
     * @param[out] value uint16_t typed value to read.
     */
    virtual void read(uint16_t *value);

    /*!
     * @brief Prototype for read uint32_t value.
     *
     * @param[out] value uint32_t typed value to read.
     */
    virtual void read(uint32_t *value);

    /*!
     * @brief Prototype for read uint64_t value.
     *
     * @param[out] value uint64_t typed value to read.
     */
    virtual void read(uint64_t *value);

    /*!
     * @brief Prototype for read float value.
     *
     * @param[out] value float typed value to read.
     */
    virtual void read(float *value);

    /*!
     * @brief Prototype for read double value.
     *
     * @param[out] value double typed value to read.
     */
    virtual void read(double *value);

    /*!
     * @brief Prototype for read uintptr value.
     *
     * @param[out] value uintptr typed value to read.
     */
    virtual void readPtr(uintptr_t *value);

    /*!
     * @brief Prototype for read string value.
     *
     * @param[out] length of string.
     * @param[out] value String value to read.
     */
    virtual void readString(uint32_t *length, char **value);

    /*!
     * @brief Prototype for read binary value.
     *
     * @param[out] length of binary.
     * @param[out] value Binary value to read.
     */
    virtual void readBinary(uint32_t *length, uint8_t **value);

    /*!
     * @brief Prototype for start read list.
     *
     * @param[out] length Length of list.
     */
    virtual void startReadList(uint32_t *length);

    /*!
     * @brief Prototype for start read union.
     *
     * @param[in] discriminator Discriminator of union.
     */
    virtual void startReadUnion(int32_t *discriminator);

    /*!
     * @brief Reads a flag indicating whether the next value is null.
     *
     * @param[in] isNull Null flag to read.
     */
    virtual void readNullFlag(bool *isNull);

    /*!
     * @brief Read an callback function id and return address of callback function.
     *
     * @param[in] callbacks Pointer to array of callbacks.
     * @param[in] callbacksCount Size of array of callbacks.
     * @param[out] callback Callback which is deserialized.
     */
    virtual void readCallback(arrayOfFunPtr callbacks, uint8_t callbacksCount, funPtr *callback);

    /*!
     * @brief Read an callback function id and return address of callback function.
     *
     * @param[in] callback1 Pointer to existing callback.
     * @param[out] callback2 Callback which is deserialized.
     */
    virtual void readCallback(funPtr callbacks1, funPtr *callback2);
    //@}
};

/*!
 * @brief Basic codec factory implements functions from codec factory.
 *
 * @ingroup infra_codec
 */
class BasicCodecFactory : public CodecFactory
{
public:
    /*!
     * @brief Return created codec.
     *
     * @return Pointer to created codec.
     */
    virtual BasicCodec *create(void) { return new (std::nothrow) BasicCodec; }

    /*!
     * @brief Dispose codec.
     *
     * @param[in] codec Codec to dispose.
     */
    virtual void dispose(Codec *codec) { delete codec; }
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__BASIC_SERIALIZATION_H_
