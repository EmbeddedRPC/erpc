/*
 * Copyright (c) 2015-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2022 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__RPMSG_LITE_RTOS_TRANSPORT_H_
#define _EMBEDDED_RPC__RPMSG_LITE_RTOS_TRANSPORT_H_

#include "erpc_crc16.hpp"
#include "erpc_message_buffer.hpp"
#include "erpc_rpmsg_lite_base.hpp"
#include "erpc_transport.hpp"

extern "C" {
#include "rpmsg_lite.h"
#include "rpmsg_queue.h"
}

/*!
 * @addtogroup rpmsg_lite_rtos_transport
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
 * @brief Transport that uses RPMsg zero copy RTOS API for interprocessor messaging.
 *
 * @ingroup rpmsg_lite_rtos_transport
 */
class RPMsgRTOSTransport : public RPMsgBase, public Transport
{
public:
    /*!
     * @brief Constructor.
     *
     * This function initializes object attributes.
     */
    RPMsgRTOSTransport(void);

    /*!
     * @brief RPMsgRTOSTransport destructor
     */
    virtual ~RPMsgRTOSTransport(void);

    /*!
     * @brief This function call RPMsg rtos init function - as RPMsg master
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
     * @brief This function call RPMsg rtos init function - as RPMsg remote
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
     * @brief Store incoming message to message buffer.
     *
     * In loop while no message come.
     *
     * @param[in] message Message buffer, to which will be stored incoming message.
     *
     * @retval kErpcStatus_ReceiveFailed Failed to receive message buffer.
     * @retval kErpcStatus_Success Successfully received all data.
     */
    virtual erpc_status_t receive(MessageBuffer *message) override;

    /*!
     * @brief Function to send prepared message.
     *
     * @param[in] message Pass message buffer to send.
     *
     * @retval kErpcStatus_SendFailed Failed to send message buffer.
     * @retval kErpcStatus_Success Successfully sent all data.
     */
    virtual erpc_status_t send(MessageBuffer *message) override;

    /*!
     * @brief This functions sets the CRC-16 implementation.
     *
     * @param[in] crcImpl Object containing crc-16 compute function.
     */
    virtual void setCrc16(Crc16 *crcImpl);

    /*!
     * @brief This functions gets the CRC-16 object.
     *
     * @return Crc16* Pointer to CRC-16 object containing crc-16 compute function.
     */
    virtual Crc16 *getCrc16(void);
    /*!
     * @brief Function to check if is message in receive queue and wait for processing.
     *
     * This function should be called before function receive() to avoid waiting for new message.
     *
     * @return True if exist received message, else false.
     */
    virtual bool hasMessage(void) { return ((rpmsg_queue_get_current_size(m_rpmsg_queue) > 0) ? true : false); }

protected:
    /* Remote device */
    struct remote_device *m_rdev;            /*!< Device which represent the second core. */
    struct rpmsg_channel *m_app_rp_chnl;     /*!< Represent connection between two device (two cores). */
    uint32_t m_dst_addr;                     /*!< Destination address used by rpmsg. */
    rpmsg_queue_handle m_rpmsg_queue;        /*!< Handle of RPMsg queue. */
    struct rpmsg_lite_endpoint *m_rpmsg_ept; /*!< Pointer to RPMsg Lite Endpoint structure. */
    Crc16 *m_crcImpl;                        //!< CRC object.
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__RPMSG_LITE_RTOS_TRANSPORT_H_
