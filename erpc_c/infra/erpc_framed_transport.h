/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__FRAMED_TRANSPORT_H_
#define _EMBEDDED_RPC__FRAMED_TRANSPORT_H_

#include "erpc_config_internal.h"
#include "erpc_message_buffer.h"
#include "erpc_transport.h"

#include <cstring>

#if !ERPC_THREADS_IS(NONE)
#include "erpc_threading.h"
#endif

/*!
 * @addtogroup infra_transport
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc {



/*! @brief Contents of the header that prefixes each message. */
struct Header
{
    uint16_t m_messageSize; //!< Size in bytes of the message, excluding the header.
    uint16_t m_crc;         //!< CRC-16 over the message data.
};

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
    /*!
     * @brief Constructor.
     */
    FramedTransport(void);

    /*!
     * @brief Codec destructor
     */
    virtual ~FramedTransport(void);

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
    virtual erpc_status_t receive(MessageBuffer *message) override;

    /*!
     * @brief Function to send prepared message.
     *
     * @param[in] message Pass message buffer to send.
     *
     * @retval kErpcStatus_Success When sending was successful.
     * @retval other Subclass may return other errors from the underlyingSend() method.
     */
    virtual erpc_status_t send(MessageBuffer *message) override;

    /*!
     * @brief This functions sets the CRC-16 implementation.
     *
     * @param[in] crcImpl Object containing crc-16 compute function.
     */
    virtual void setCrc16(Crc16 *crcImpl) override;

protected:
    Crc16 *m_crcImpl; /*!< CRC object. */

#if !ERPC_THREADS_IS(NONE)
    Mutex m_sendLock;    //!< Mutex protecting send.
    Mutex m_receiveLock; //!< Mutex protecting receive.
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
    virtual erpc_status_t underlyingSend(const uint8_t *data, uint32_t size) = 0;

    /*!
     * @brief Subclasses must implement this function to receive data.
     *
     * @param[inout] data Preallocated buffer for receiving data.
     * @param[in] size Size of data to read.
     *
     * @retval kErpcStatus_Success When data was read successfully.
     * @retval kErpcStatus_Fail When reading data ends with error.
     */
    virtual erpc_status_t underlyingReceive(uint8_t *data, uint32_t size) = 0;
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__FRAMED_TRANSPORT_H_
