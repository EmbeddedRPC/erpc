/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__RPMSG_LITE_TTY_RTOS_TRANSPORT_H_
#define _EMBEDDED_RPC__RPMSG_LITE_TTY_RTOS_TRANSPORT_H_

#include "erpc_crc16.hpp"
#include "erpc_framed_transport.hpp"
#include "erpc_message_buffer.hpp"
#include "erpc_rpmsg_lite_base.hpp"

extern "C" {
#include "rpmsg_lite.h"
#include "rpmsg_queue.h"
}

/*!
 * @addtogroup rpmsg_tty_rtos_transport
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
 * @brief Transport that uses RPMsg zero copy RTOS API for interprocessor
 * messaging.
 *
 * @ingroup rpmsg_tty_rtos_transport
 */
class RPMsgTTYRTOSTransport : public FramedTransport, public RPMsgBase
{
public:
    /*!
     * @brief Constructor.
     *
     * This function initializes object attributes.
     */
    RPMsgTTYRTOSTransport(void);

    /*!
     * @brief RPMsgRTOSTransport destructor
     */
    virtual ~RPMsgTTYRTOSTransport(void);

    /*!
     * @brief This function call RPMsg rtos init function - as RPMsg master
     *
     * @param[in] src_addr Source address.
     * @param[in] dst_addr Destination address.
     * @param[in] base_address RPMsg base address in the shared memory.
     * @param[in] length RPMsg shared memory region length.
     * @param[in] rpmsg_link_id Selection between what cores the communication
     * will occur.
     *
     * @retval kErpcStatus_Success When rpmsg init function was executed
     * successfully.
     * @retval kErpcStatus_InitFailed When rpmsg init function wasn't executed
     * successfully.
     */
    virtual erpc_status_t init(uint32_t src_addr, uint32_t dst_addr, void *base_address, uint32_t length,
                               uint32_t rpmsg_link_id);

    /*!
     * @brief This function call RPMsg rtos init function - as RPMsg remote
     *
     * @param[in] src_addr Source address.
     * @param[in] dst_addr Destination address.
     * @param[in] base_address RPMsg base address in the shared memory.
     * @param[in] rpmsg_link_id Selection between what cores the communication
     * will occur.
     * @param[in] ready_cb Callback called after RPMsg init is done and the core
     * is ready.
     * @param[in] nameservice_name Name of the nameservice channel to be announced
     *                             to the other core.
     *
     * @retval kErpcStatus_Success When rpmsg init function was executed
     * successfully.
     * @retval kErpcStatus_InitFailed When rpmsg init function wasn't executed
     * successfully.
     */
    virtual erpc_status_t init(uint32_t src_addr, uint32_t dst_addr, void *base_address, uint32_t rpmsg_link_id,
                               void (*ready_cb)(void), char *nameservice_name);

protected:
    uint32_t m_dst_addr;                     /*!< Destination address used by rpmsg. */
    rpmsg_queue_handle m_rpmsg_queue;        /*!< Handle of RPMsg queue. */
    struct rpmsg_lite_endpoint *m_rpmsg_ept; /*!< Pointer to RPMsg Lite Endpoint structure. */

    /*!
     * @brief Adds ability to framed transport to overwrite MessageBuffer when sending data.
     *
     * Usually we don't want to do that.
     *
     * @param message MessageBuffer to send.
     * @param size size of message to send.
     * @param offset data start address offset
     *
     * @return erpc_status_t kErpcStatus_Success when it finished successful otherwise error.
     */
    virtual erpc_status_t underlyingSend(MessageBuffer *message, uint32_t size, uint32_t offset) override;

    /*!
     * @brief Adds ability to framed transport to overwrite MessageBuffer when receiving data.
     *
     * Usually we don't want to do that.
     *
     * @param message MessageBuffer to send.
     * @param size size of message to send.
     * @param offset data start address offset
     *
     * @return erpc_status_t kErpcStatus_Success when it finished successful otherwise error.
     */
    virtual erpc_status_t underlyingReceive(MessageBuffer *message, uint32_t size, uint32_t offset) override;

    /*!
     * @brief This function read data.
     *
     * @param[inout] data Preallocated buffer for receiving data.
     * @param[in] size Size of data to read.
     *
     * @retval #kErpcStatus_Success When data was read successfully.
     * @retval #kErpcStatus_ReceiveFailed When reading data ends with error.
     * @retval #kErpcStatus_ConnectionClosed Peer closed the connection.
     */
    virtual erpc_status_t underlyingReceive(uint8_t *data, uint32_t size) override;

    /*!
     * @brief This function writes data.
     *
     * @param[in] data Buffer to send.
     * @param[in] size Size of data to send.
     *
     * @retval #kErpcStatus_Success When data was written successfully.
     * @retval #kErpcStatus_SendFailed When writing data ends with error.
     * @retval #kErpcStatus_ConnectionClosed Peer closed the connection.
     */
    virtual erpc_status_t underlyingSend(const uint8_t *data, uint32_t size) override;
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__RPMSG_LITE_TTY_RTOS_TRANSPORT_H_
