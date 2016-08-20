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

#ifndef _EMBEDDED_RPC__BASIC_SERIALIZATION_H_
#define _EMBEDDED_RPC__BASIC_SERIALIZATION_H_

#include "codec.h"
#include <new>

#if !(__embedded_cplusplus)
using namespace std;
#endif

/*!
 * @addtogroup infra_codec
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc
{
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
    static const uint32_t kBasicCodecVersion = 1; /*!< Codec version. */

    BasicCodec()
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
     *                    received message is reply for current request.
     *
     * @retval kErpcStatus_Success or write function.
     */
    virtual status_t startWriteMessage(message_type_t type, uint32_t service, uint32_t request, uint32_t sequence);

    /*!
     * @brief Prototype for write end of message.
     *
     * @retval kErpcStatus_Success.
     */
    virtual status_t endWriteMessage();

    /*!
     * @brief Prototype for write boolean value.
     *
     * @param[in] value Boolean typed value to write.
     *
     * @return depends on cursor write function.
     */
    virtual status_t write(bool value);

    /*!
     * @brief Prototype for write int8_t value.
     *
     * @param[in] value int8_t typed value to write.
     *
     * @return depends on cursor write function.
     */
    virtual status_t write(int8_t value);

    /*!
     * @brief Prototype for write int16_t value.
     *
     * @param[in] value int16_t typed value to write.
     *
     * @return depends on cursor write function.
     */
    virtual status_t write(int16_t value);

    /*!
     * @brief Prototype for write int32_t value.
     *
     * @param[in] value int32_t typed value to write.
     *
     * @return depends on cursor write function.
     */
    virtual status_t write(int32_t value);

    /*!
     * @brief Prototype for write int64_t value.
     *
     * @param[in] value int64_t typed value to write.
     *
     * @return depends on cursor write function.
     */
    virtual status_t write(int64_t value);

    /*!
     * @brief Prototype for write uint8_t value.
     *
     * @param[in] value uint8_t typed value to write.
     *
     * @return depends on cursor write function.
     */
    virtual status_t write(uint8_t value);

    /*!
     * @brief Prototype for write uint16_t value.
     *
     * @param[in] value uint16_t typed value to write.
     *
     * @return depends on cursor write function.
     */
    virtual status_t write(uint16_t value);

    /*!
     * @brief Prototype for write uint32_t value.
     *
     * @param[in] value uint32_t typed value to write.
     *
     * @return depends on cursor write function.
     */
    virtual status_t write(uint32_t value);

    /*!
     * @brief Prototype for write uint64_t value.
     *
     * @param[in] value uint64_t typed value to write.
     *
     * @return depends on cursor write function.
     */
    virtual status_t write(uint64_t value);

    /*!
     * @brief Prototype for write float value.
     *
     * @param[in] value float typed value to write.
     *
     * @return depends on cursor write function.
     */
    virtual status_t write(float value);

    /*!
     * @brief Prototype for write double value.
     *
     * @param[in] value double typed value to write.
     *
     * @return depends on cursor write function.
     */
    virtual status_t write(double value);

    /*!
     * @brief Prototype for write string value.
     *
     * @param[in] length of string.
     * @param[in] value string value to write.
     *
     * @return depends on writeBinary function.
     */
    virtual status_t writeString(uint32_t length, const char *value);

    /*!
     * @brief Prototype for write binary value.
     *
     * @param[in] length of binary.
     * @param[in] value Binary value to write.
     *
     * @return depends on write function and cursor write function.
     */
    virtual status_t writeBinary(uint32_t length, const uint8_t *value);

    /*!
     * @brief Prototype for start write list.
     *
     * @param[in] length Length of list.
     *
     * @return depends on write function.
     */
    virtual status_t startWriteList(uint32_t length);

    /*!
     * @brief Prototype for end write list.
     *
     * @retval kErpcStatus_Success.
     */
    virtual status_t endWriteList();

    /*!
     * @brief Prototype for start write structure.
     *
     * @retval kErpcStatus_Success.
     */
    virtual status_t startWriteStruct();

    /*!
     * @brief Prototype for end write structure.
     *
     * @retval kErpcStatus_Success.
     */
    virtual status_t endWriteStruct();

    /*!
     * @brief Writes a flag indicating whether the next value is null.
     *
     * @retval kErpcStatus_Success
     */
    virtual status_t writeNullFlag(bool isNull);
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
     *
     * @retval kErpcStatus_Success
     * @retval kErpcStatus_InvalidMessageVersion
     */
    virtual status_t startReadMessage(message_type_t *type, uint32_t *service, uint32_t *request, uint32_t *sequence);

    /*!
     * @brief Prototype for read end of message.
     *
     * @retval kErpcStatus_Success.
     */
    virtual status_t endReadMessage();

    /*!
     * @brief Prototype for read boolean value.
     *
     * @param[out] value Boolean typed value to read.
     *
     * @return Based on cursor read function.
     */
    virtual status_t read(bool *value);

    /*!
     * @brief Prototype for read int8_t value.
     *
     * @param[out] value int8_t typed value to read.
     *
     * @return Based on cursor read function.
     */
    virtual status_t read(int8_t *value);

    /*!
     * @brief Prototype for read int16_t value.
     *
     * @param[out] value int16_t typed value to read.
     *
     * @return Based on cursor read function.
     */
    virtual status_t read(int16_t *value);

    /*!
     * @brief Prototype for read int32_t value.
     *
     * @param[out] value int32_t typed value to read.
     *
     * @return Based on cursor read function.
     */
    virtual status_t read(int32_t *value);

    /*!
     * @brief Prototype for read int64_t value.
     *
     * @param[out] value int64_t typed value to read.
     *
     * @return Based on cursor read function.
     */
    virtual status_t read(int64_t *value);

    /*!
     * @brief Prototype for read uint8_t value.
     *
     * @param[out] value uint8_t typed value to read.
     *
     * @return Based on cursor read function.
     */
    virtual status_t read(uint8_t *value);

    /*!
     * @brief Prototype for read uint16_t value.
     *
     * @param[out] value uint16_t typed value to read.
     *
     * @return Based on cursor read function.
     */
    virtual status_t read(uint16_t *value);

    /*!
     * @brief Prototype for read uint32_t value.
     *
     * @param[out] value uint32_t typed value to read.
     *
     * @return Based on cursor read function.
     */
    virtual status_t read(uint32_t *value);

    /*!
     * @brief Prototype for read uint64_t value.
     *
     * @param[out] value uint64_t typed value to read.
     *
     * @return Based on cursor read function.
     */
    virtual status_t read(uint64_t *value);

    /*!
     * @brief Prototype for read float value.
     *
     * @param[out] value float typed value to read.
     *
     * @return Based on cursor read function.
     */
    virtual status_t read(float *value);

    /*!
     * @brief Prototype for read double value.
     *
     * @param[out] value double typed value to read.
     *
     * @return Based on cursor read function.
     */
    virtual status_t read(double *value);

    /*!
     * @brief Prototype for read string value.
     *
     * @param[out] length of string.
     * @param[out] value String value to read.
     *
     * @return Based on readBinary function.
     */
    virtual status_t readString(uint32_t *length, char **value);

    /*!
     * @brief Prototype for read binary value.
     *
     * @param[out] length of binary.
     * @param[out] value Binary value to read.
     *
     * @retval kErpcStatus_Success or depends on read function.
     */
    virtual status_t readBinary(uint32_t *length, uint8_t **value);

    /*!
     * @brief Prototype for start read list.
     *
     * @param[out] length Length of list.
     *
     * @return depends on read function.
     */
    virtual status_t startReadList(uint32_t *length);

    /*!
     * @brief Prototype for end read list.
     *
     * @retval kErpcStatus_Success.
     */
    virtual status_t endReadList();

    /*!
     * @brief Prototype for start read structure.
     *
     * @retval kErpcStatus_Success.
     */
    virtual status_t startReadStruct();

    /*!
     * @brief Prototype for end read structure.
     *
     * @retval kErpcStatus_Success.
     */
    virtual status_t endReadStruct();

    /*!
     * @brief Reads a flag indicating whether the next value is null.
     *
     * @retval kErpcStatus_Success
     */
    virtual status_t readNullFlag(bool *isNull);
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
    virtual BasicCodec *create() { return new (nothrow) BasicCodec; }
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
