/*
 * The Clear BSD License
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 * that the following conditions are met:
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
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
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

#ifndef _EMBEDDED_RPC__MESSAGE_BUFFER_H_
#define _EMBEDDED_RPC__MESSAGE_BUFFER_H_

#include "erpc_common.h"
#include <cstddef>
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
 * @brief Represents a memory buffer containing a message.
 *
 * The MessageBuffer object does not own the buffer memory. It simply provides an interface
 * to accessing that memory in a convenient manner.
 *
 * @ingroup infra_codec
 */
class MessageBuffer
{
public:
    /*!
     * @brief Constructor.
     *
     * This function initializes object attributes.
     */
    MessageBuffer(void)
    : m_buf(0)
    , m_len(0)
    , m_used(0)
    {
    }

    /*!
     * @brief Constructor.
     *
     * This function initializes object attributes.
     *
     * param[in] buffer Pointer to buffer.
     * param[in] length Length of buffer.
     */
    MessageBuffer(uint8_t *buffer, uint16_t length)
    : m_buf(buffer)
    , m_len(length)
    , m_used(0)
    {
    }

    /*!
     * @brief This function set new buffer and his length.
     *
     * This function set buffer to read/write data.
     *
     * @param[in] buffer Pointer to another buffer to read/write data.
     * @param[in] length Length of buffer.
     */
    void set(uint8_t *buffer, uint16_t length)
    {
        m_buf = buffer;
        m_len = length;
        m_used = 0;
    }

    /*!
     * @brief This function returns pointer to buffer to read/write.
     *
     * @return Pointer to buffer to read/write.
     */
    uint8_t *get(void) { return m_buf; }

    /*!
     * @brief This function returns pointer to buffer to read/write.
     *
     * @return Pointer to buffer to read/write.
     */
    const uint8_t *get(void) const { return m_buf; }

    /*!
     * @brief This function returns length of buffer.
     *
     * @return Length of buffer.
     */
    uint16_t getLength(void) const { return m_len; }

    /*!
     * @brief This function returns length of used space of buffer.
     *
     * @return Length of used space of buffer.
     */
    uint16_t getUsed(void) const { return m_used; }

    /*!
     * @brief This function returns length of free space of buffer.
     *
     * @return Length of free space of buffer.
     */
    uint16_t getFree(void) const { return m_len - m_used; }

    /*!
     * @brief This function sets length of used space of buffer.
     *
     * @param[in] used Length of used space of buffer.
     */
    void setUsed(uint16_t used) { m_used = used; }

    /*!
     * @brief This function read data from local buffer.
     *
     * @param[in] offset Offset in local buffer.
     * @param[inout] data Given buffer to save read data.
     * @param[in] length Length of data to read.
     *
     * @return Status from reading data.
     */
    erpc_status_t read(uint16_t offset, void *data, uint32_t length);

    /*!
     * @brief This function write data to local buffer.
     *
     * @param[in] offset Offset in local buffer.
     * @param[inout] data Given buffer from which are copied data.
     * @param[in] length Length of data to write.
     *
     * @return Status from reading data.
     */
    erpc_status_t write(uint16_t offset, const void *data, uint32_t length);

    /*!
     * @brief This function copy given message buffer to local instance.
     *
     * @param[in] other MesageBuffer to copy.
     *
     * @return Status from reading data.
     */
    erpc_status_t copy(const MessageBuffer *other);

    /*!
     * @brief This function swap message buffer attributes between given instance and local instance.
     *
     * @param[in] other MesageBuffer to swap.
     */
    void swap(MessageBuffer *other);

    /*!
     * @brief Casting operator return local buffer.
     */
    operator uint8_t *(void) { return m_buf; }

    /*!
     * @brief Casting operator return local buffer.
     */
    operator const uint8_t *(void)const { return m_buf; }

    /*!
     * @brief Array operator return value of buffer at given index.
     *
     * @param[in] index Index in buffer.
     */
    uint8_t &operator[](int index) { return m_buf[index]; }

    /*!
     * @brief Array operator return value of buffer at given index.
     *
     * @param[in] index Index in buffer.
     */
    const uint8_t &operator[](int index) const { return m_buf[index]; }

    /*!
     * @brief Cursor within a MessageBuffer.
     */
    class Cursor
    {
    public:
        /*!
         * @brief Constructor.
         *
         * This function initializes object attributes.
         */
        Cursor(void)
        : m_buffer(NULL)
        , m_pos(NULL)
        , m_remaining(0)
        {
        }

        /*!
         * @brief Constructor.
         *
         * This function initializes object attributes.
         *
         * param[in] buffer MessageBuffer for sending/receiving.
         */
        Cursor(MessageBuffer *buffer)
        : m_buffer(buffer)
        , m_pos(buffer->get())
        , m_remaining(buffer->getLength())
        {
        }

        /*!
         * @brief Set message buffer.
         *
         * @param[in] buffer Message buffer to set.
         */
        void set(MessageBuffer *buffer);

        /*!
         * @brief Return position in buffer.
         *
         * Return position, where it last write/read.
         *
         * @return Return position in buffer.
         */
        uint8_t *get(void) { return m_pos; }

        /*!
         * @brief Return position in buffer.
         *
         * Return position, where it last write/read.
         *
         * @return Return position in buffer.
         */
        const uint8_t *get(void) const { return m_pos; }

        /*!
         * @brief Return remaining free space in current buffer.
         *
         * @return Remaining free space in current buffer.
         */
        uint16_t getRemaining(void) const { return m_remaining; }

        /*!
         * @brief Read data from current buffer.
         *
         * @param[out] data Pointer to value, where copy read data.
         * @param[in] length How much bytes need be read.
         *
         * @retval kErpcStatus_Success
         * @retval kErpcStatus_BufferOverrun
         */
        erpc_status_t read(void *data, uint32_t length);

        /*!
         * @brief Read data from current buffer.
         *
         * @param[out] data Pointer to value to be sent.
         * @param[in] length How much bytes need be wrote.
         *
         * @retval kErpcStatus_Success
         * @retval kErpcStatus_BufferOverrun
         */
        erpc_status_t write(const void *data, uint32_t length);

        /*!
         * @brief Casting operator return local buffer.
         */
        operator uint8_t *(void) { return m_pos; }

        /*!
         * @brief Casting operator return local buffer.
         */
        operator const uint8_t *(void)const { return m_pos; }

        /*!
         * @brief Array operator return value of buffer at given index.
         *
         * @param[in] index Index in buffer.
         */
        uint8_t &operator[](int index) { return m_pos[index]; }

        /*!
         * @brief Array operator return value of buffer at given index.
         *
         * @param[in] index Index in buffer.
         */
        const uint8_t &operator[](int index) const { return m_pos[index]; }

        /*!
         * @brief Sum operator return local buffer.
         *
         * @param[in] n Summing with n.
         *
         * @return Current cursor instance.
         */
        Cursor &operator+=(uint16_t n)
        {
            m_pos += n;
            m_remaining -= n;
            return *this;
        }

        /*!
         * @brief Substract operator return local buffer.
         *
         * @param[in] n Substracting with n.
         *
         * @return Current cursor instance.
         */
        Cursor &operator-=(uint16_t n)
        {
            m_pos -= n;
            m_remaining += n;
            return *this;
        }

        /*!
         * @brief Sum +1 operator.
         *
         * @return Current cursor instance.
         */
        Cursor &operator++(void)
        {
            ++m_pos;
            --m_remaining;
            return *this;
        }

        /*!
         * @brief Substract -1 operator.
         *
         * @return Current cursor instance.
         */
        Cursor &operator--(void)
        {
            --m_pos;
            ++m_remaining;
            return *this;
        }

    private:
        MessageBuffer *m_buffer; /*!< Buffer for reading or writing data. */
        uint8_t *m_pos;          /*!< Position in buffer, where it last write/read */
        uint16_t m_remaining;    /*!< Remaining space in buffer. */
    };

private:
    uint8_t *volatile m_buf;  /*!< Buffer used to read write data. */
    uint16_t volatile m_len;  /*!< Length of buffer. */
    uint16_t volatile m_used; /*!< Used buffer bytes. */
};

/*!
 *  @brief Abstract interface for message buffer factory.
 *
 * @ingroup infra_codec
 */
class MessageBufferFactory
{
public:
    /*!
     * @brief Constructor.
     *
     * This function initializes object attributes.
     */
    MessageBufferFactory(void) {}

    /*!
     * @brief ClientManager destructor
     */
    virtual ~MessageBufferFactory(void) {}

    /*!
     * @brief This function creates new message buffer.
     *
     * @return New created MessageBuffer.
     */
    virtual MessageBuffer create(void) = 0;

    /*!
     * @brief This function inform server if it has to create buffer for received message.
     *
     * @return Has to return TRUE when server need create buffer for receiving message.
     */
    virtual bool createServerBuffer(void) { return true; }

    /*!
     * @brief This function is preparing output buffer on server side.
     *
     * This function do decision if this function want reuse buffer, or use new buffer.
     * In case of using new buffer function has to free given buffer.
     *
     * @param[in] message MessageBuffer which can be reused.
     */
    virtual erpc_status_t prepareServerBufferForSend(MessageBuffer *message);

    /*!
     * @brief This function disposes message buffer.
     *
     * @param[in] buf MessageBuffer to dispose.
     */
    virtual void dispose(MessageBuffer *buf) = 0;
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__MESSAGE_BUFFER_H_
