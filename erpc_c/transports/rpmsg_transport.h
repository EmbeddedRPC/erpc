/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
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

#ifndef _EMBEDDED_RPC__RPMSG_TRANSPORT_H_
#define _EMBEDDED_RPC__RPMSG_TRANSPORT_H_

#include "transport.h"
#include "message_buffer.h"
#include "static_queue.h"

extern "C" {
#include "rpmsg.h"
}

/*!
 * @addtogroup rpmsg_transport
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

enum
{
    kRpmsgMessageBufferSize = RPMSG_BUFFER_SIZE,
    kInitCountMessageBuffers = 2,
};

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc
{
/*!
 * @brief Transport that uses RPMsg for interprocessor messaging.
 *
 * @ingroup rpmsg_transport
 */
class RPMsgTransport : public Transport
{
public:
    /*!
     * @brief Constructor.
     *
     * This function initializes object attributes.
     */
    RPMsgTransport();

    /*!
     * @brief Codec destructor
     */
    virtual ~RPMsgTransport();

    /*!
     * @brief Initialization of RPMsgTransport layer.
     *
     * Call init() for RPMsg. Create buffers for receiving messages.
     *
     * @param[in] dev_id RPMsg remote device for which driver is to be initialized.
     *
     * @param[in] role Role of the other device.
     *
     * @return kErpcStatus_Success when initialization was successful, else kErpcStatus_Fail.
     */
    virtual status_t init(int dev_id, int role);

    /*!
     * @brief Set message to first received message.
     *
     * In loop while no message come.
     *
     * @param[in] message Message buffer, to which will be stored incoming message.
     *
     * @return kErpcStatus_Success
     */
    virtual status_t receive(MessageBuffer *message);

    /*!
     * @brief Function to send prepared message.
     *
     * @param[in] message Pass message buffer to send.
     *
     * @return kErpcStatus_Success when all buffers were send, else kErpcStatus_Fail.
     */
    virtual status_t send(const MessageBuffer *message);

    /*!
     * @brief Function to check if is message in receive queue and wait for processing.
     *
     * This function should be called before function receive() to avoid waiting for new message.
     *
     * @return True if exist received message, else false.
     */
    virtual bool hasMessage() { return messageQueue.size(); }
protected:
    static StaticQueue<MessageBuffer *>
        messageQueue; /*!< Received messages. Queue of messages with buffers filled in rpmsg callback. */
    static StaticQueue<MessageBuffer *>
        freeMessageQueue; /*!< Pre-allocated free messages. Queue of messages with free buffers. */

    /* Remote device */
    static struct remote_device *m_rdev;                 /*!< Device which represent the second core. */
    static struct rpmsg_endpoint *m_rp_ept;              /*!< Communication end point.*/
    static struct rpmsg_channel *volatile m_app_rp_chnl; /*!< Represent connection between two device (two cores). */

    /* Application provided callbacks */
    /*!
     * @brief This callback gets invoked when the remote channel is created.
     *
     * @param[] rp_chnl Created channel.
     */
    static void rpmsg_channel_created(struct rpmsg_channel *rp_chnl);

    /*!
     * @brief This callback gets invoked when the remote channel is deleted.
     *
     * @param[] rp_chnl Deleted channel.
     */
    static void rpmsg_channel_deleted(struct rpmsg_channel *rp_chnl);

    /*!
     * @brief This is the RPMsg read callback.
     *
     * This callback is invoked when new data are received. In this callback are
     * data copied to preallocated message buffer, which is added to queue for next
     * processing.
     */
    static void rpmsg_read_cb(struct rpmsg_channel *rp_chnl, void *data, int len, void *pric, unsigned long src);

private:
    /*!
     * @brief This function clear all objects in message queue.
     *
     * @param[in] messageQueue to clear.
     *
     * This function clears all message buffers from queue. Every message buffer
     * is deallocated.
     */
    static void clearMessageQueue(StaticQueue<MessageBuffer *> *messageQueue);
};

/*!
 * @brief Message buffer factory used with RPMsg transport layer.
 */
class RPMsgMessageBufferFactory : public MessageBufferFactory
{
public:
    /*!
     * @brief Constructor.
     */
    RPMsgMessageBufferFactory() {}
    /*!
     * @brief CodecFactory destructor
     */
    virtual ~RPMsgMessageBufferFactory() {}
    /*!
     * @brief This function creates new message buffer.
     *
     * @return MessageBuffer New created MessageBuffer.
     */
    virtual MessageBuffer create();

    /*!
     * @brief This function disposes message buffer.
     *
     * @param[in] buf MessageBuffer to dispose.
     */
    virtual void dispose(MessageBuffer *buf);
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__RPMSG_TRANSPORT_H_
