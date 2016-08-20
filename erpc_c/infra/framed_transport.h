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

#ifndef _EMBEDDED_RPC__FRAMED_TRANSPORT_H_
#define _EMBEDDED_RPC__FRAMED_TRANSPORT_H_

#include <stdint.h>
#include <cstring>
#include "transport.h"
#include "message_buffer.h"
#include "erpc_threading.h"

/*!
 * @addtogroup infra_transport
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc
{
class MessageBuffer;

/*!
 * @brief Base class for framed transport layers.
 *
 * This class adds simple framing to the data transmitted and received on the
 * communications channel. This allows the transport to perform reads and writes
 * of a size known in advance. Subclasses must implement the underlyingSend() and
 * underlyingReceive() methods to actually transmit and receive data.
 *
 * Frames have a maximum size of 64kB, as a 16-bit frame size is used.
 *
 * @note This implementation currently assumes both sides of the communications channel
 *  are the same endianness.
 *
 * The frame header includes a CRC-16 over the data for integrity checking. This class
 * includes a default CRC-16 implementation that is optimized for code size, but is
 * relatively slow. If a faster implementation is desired, you can pass the new CRC
 * function to setCRCFunction().
 *
 * @ingroup infra_transport
 */
class FramedTransport : public Transport
{
public:
    //! @brief Function type for a CRC-16 implementation.
    typedef uint16_t (*compute_crc_t)(const uint8_t *data, uint32_t lengthInBytes);

    /*!
     * @brief Constructor.
     *
     * Sets the CRC-16 implementation to the internal function.
     */
    FramedTransport();

    /*!
     * @brief Codec destructor
     */
    virtual ~FramedTransport();

    /*!
     * @brief Receives an entire message.
     *
     * The frame header and message data are received. The CRC-16 in the frame header is
     * compared with the computed CRC. If the received CRC is invalid, #kErpcStatus_Fail
     * will be returned.
     *
     * The @a message is only filled with the message data, not the frame header.
     *
     * This function is blocking.
     *
     * @param[in] message Message buffer, to which will be stored incoming message.
     *
     * @retval kErpcStatus_Success When receiving was successful.
     * @retval kErpcStatus_CrcCheckFailed When receiving failed.
     * @retval other Subclass may return other errors from the underlyingReceive() method.
     */
    virtual status_t receive(MessageBuffer *message);

    /*!
     * @brief Function to send prepared message.
     *
     * @param[in] message Pass message buffer to send.
     *
     * @retval kErpcStatus_Success When sending was successful.
     * @retval other Subclass may return other errors from the underlyingSend() method.
     */
    virtual status_t send(const MessageBuffer *message);

    //! @brief Override the CRC-16 implementation.
    void setCRCFunction(compute_crc_t crcFunction);

protected:
    compute_crc_t m_crcImpl; //!< CRC function.

#if ERPC_THREADS
    Mutex m_sendLock;       //!< Mutex protecting send.
    Mutex m_receiveLock;    //!< Mutex protecting receive.
#endif

    /*!
     * @brief Subclasses must implement this function to send data.
     *
     * @param[in] data Buffer to send.
     * @param[in] size Size of data to send.
     *
     * @retval kErpcStatus_Success When data was written successfully.
     * @retval kErpcStatus_Fail When writing data ends with error.
     */
    virtual status_t underlyingSend(const uint8_t *data, uint32_t size) = 0;

    /*!
     * @brief Subclasses must implement this function to receive data.
     *
     * @param[inout] data Preallocated buffer for receiving data.
     * @param[in] size Size of data to read.
     *
     * @retval kErpcStatus_Success When data was read successfully.
     * @retval kErpcStatus_Fail When reading data ends with error.
     */
    virtual status_t underlyingReceive(uint8_t *data, uint32_t size) = 0;

    /*! @brief Contents of the header that prefixes each message. */
    struct Header
    {
        uint16_t m_messageSize; //!< Size in bytes of the message, excluding the header.
        uint16_t m_crc;         //!< CRC-16 over the message data.
    };

    //! @brief Compute a ITU-CCITT CRC-16 over the provided data.
    //!
    //! This implementation is slow but small in size.
    static uint16_t computeCRC16(const uint8_t *data, uint32_t lengthInBytes);
};

} // namespace

/*! @} */

#endif // _EMBEDDED_RPC__FRAMED_TRANSPORT_H_
