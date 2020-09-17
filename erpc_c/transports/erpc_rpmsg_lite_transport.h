/*
 * Copyright (c) 2015-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__RPMSG_LITE_TRANSPORT_H_
#define _EMBEDDED_RPC__RPMSG_LITE_TRANSPORT_H_

#include "erpc_config_internal.h"
#include "erpc_message_buffer.h"
#include "erpc_rpmsg_lite_base_transport.h"
#include "erpc_static_queue.h"
#include "rpmsg_lite.h"

/*!
 * @addtogroup rpmsg_lite_transport
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
class RPMsgTransport : public RPMsgBaseTransport
{
public:
    /*!
     * @brief Constructor.
     *
     * This function initializes object attributes.
     */
    RPMsgTransport(void);

    /*!
     * @brief Codec destructor
     */
    virtual ~RPMsgTransport(void);

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
    virtual erpc_status_t init(uint32_t src_addr, uint32_t dst_addr, void *base_address, uint32_t length,
                               uint32_t rpmsg_link_id);

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
     * @param[in] nameservice_name Name of the nameservice channel to be announced
     *                             to the other core.
     *
     * @retval kErpcStatus_Success When rpmsg init function was executed successfully.
     * @retval kErpcStatus_InitFailed When rpmsg init function wasn't executed successfully.
     */
    virtual erpc_status_t init(uint32_t src_addr, uint32_t dst_addr, void *base_address, uint32_t rpmsg_link_id,
                               void (*ready_cb)(void), char *nameservice_name);

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
    virtual bool hasMessage(void) { return m_messageQueue.size(); }

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
    static int32_t rpmsg_read_cb(void *payload, uint32_t payload_len, uint32_t src, void *priv);

    StaticQueue<MessageBuffer, ERPC_DEFAULT_BUFFERS_COUNT>
        m_messageQueue; /*!< Received messages. Queue of messages with buffers filled in rpmsg callback. */

    uint32_t m_dst_addr;                                      /*!< Destination address used by rpmsg. */
    struct rpmsg_lite_ept_static_context m_rpmsg_ept_context; /*!< RPMsg Lite Endpoint static context. */
    struct rpmsg_lite_endpoint *m_rpmsg_ept;                  /*!< Pointer to RPMsg Lite Endpoint structure. */

    static struct rpmsg_lite_instance s_rpmsg_ctxt; /*!< Context for RPMsg Lite stack instance. */
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__RPMSG_LITE_TRANSPORT_H_
