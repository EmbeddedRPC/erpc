/*
 * Copyright (c) 2015-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
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

#ifndef _EMBEDDED_RPC__RPMSG_LITE_ZC_TRANSPORT_H_
#define _EMBEDDED_RPC__RPMSG_LITE_ZC_TRANSPORT_H_

#include "erpc_config_internal.h"
#include "message_buffer.h"
#include "rpmsg_lite.h"
#include "rpmsg_lite_zc_base_transport.h"
#include "static_queue.h"

/*!
 * @addtogroup rpmsg_lite_zc_transport
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc {
/*!
 * @brief Transport that uses RPMsg zero copy API for interprocessor messaging.
 *
 * @ingroup rpmsg_lite_transport
 */
class RPMsgZCTransport : public RPMsgZCBaseTransport
{
public:
    /*!
     * @brief Constructor.
     *
     * This function initializes object attributes.
     */
    RPMsgZCTransport();

    /*!
     * @brief Codec destructor
     */
    virtual ~RPMsgZCTransport();

    /*!
     * @brief Initialization of RPMsgTransport layer - as RPMsg master
     *
     * Call init() for RPMsg. Create buffers for receiving messages.
     *
     * @param[in] src_addr Source address.
     * @param[in] dst_addr Destination address.
     * @param[in] base_address RPMsg base address in the shared memory.
     * @param[in] length RPMsg shared memory region length.
     * @param[in] rpmsg_link_id Selection between what cores the communication will occur.
     *
     * @retval kErpcStatus_Success When rpmsg init function was executed successfully.
     * @retval kErpcStatus_InitFailed When rpmsg init function wasn't executed successfully.
     */
    virtual erpc_status_t init(
        unsigned long src_addr, unsigned long dst_addr, void *base_address, unsigned long length, int rpmsg_link_id);

    /*!
     * @brief Initialization of RPMsgTransport layer - as RPMsg remote
     *
     * Call init() for RPMsg. Create buffers for receiving messages.
     *
     * @param[in] src_addr Source address.
     * @param[in] dst_addr Destination address.
     * @param[in] base_address RPMsg base address in the shared memory.
     * @param[in] rpmsg_link_id Selection between what cores the communication will occur.
     * @param[in] ready_cb Callback called after RPMsg init is done and the core is ready.
     *
     * @retval kErpcStatus_Success When rpmsg init function was executed successfully.
     * @retval kErpcStatus_InitFailed When rpmsg init function wasn't executed successfully.
     */
    virtual erpc_status_t init(
        unsigned long src_addr, unsigned long dst_addr, void *base_address, int rpmsg_link_id, void (*ready_cb)(void));

    /*!
     * @brief Set message to first received message.
     *
     * In loop while no message come.
     *
     * @param[in] message Message buffer, to which will be stored incoming message.
     *
     * @return kErpcStatus_Success
     */
    virtual erpc_status_t receive(MessageBuffer *message);

    /*!
     * @brief Function to send prepared message.
     *
     * @param[in] message Pass message buffer to send.
     *
     * @retval kErpcStatus_SendFailed Failed to send message buffer.
     * @retval kErpcStatus_Success Successfully sent all data.
     */
    virtual erpc_status_t send(MessageBuffer *message);

    /*!
     * @brief Function to check if is message in receive queue and wait for processing.
     *
     * This function should be called before function receive() to avoid waiting for new message.
     *
     * @return True if exist received message, else false.
     */
    virtual bool hasMessage() { return m_messageQueue.size(); }

protected:
    /*!
     * @brief RPMSG callback for receiving data.
     *
     * This function is used as RPMSG receive callback in which is copied message
     * to free message buffer.
     *
     * @param payload RPMSG data to receive.
     * @param payload_len Length of data.
     * @param src Source endpoint address.
     * @param priv Callback data.
     *
     * @return
     */
    static int rpmsg_read_cb(void *payload, int payload_len, unsigned long src, void *priv);

    StaticQueue<MessageBuffer, ERPC_DEFAULT_BUFFERS_COUNT>
        m_messageQueue; /*!< Received messages. Queue of messages with buffers filled in rpmsg callback. */

    unsigned long m_dst_addr;                                 /*!< Destination address used by rpmsg. */
    struct rpmsg_lite_ept_static_context m_rpmsg_ept_context; /*!< RPMsg Lite Endpoint static context. */
    struct rpmsg_lite_endpoint *m_rpmsg_ept;                  /*!< Pointer to RPMsg Lite Endpoint structure. */

    static struct rpmsg_lite_instance s_rpmsg_ctxt; /*!< Context for RPMsg Lite stack instance. */
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__RPMSG_LITE_ZC_TRANSPORT_H_
