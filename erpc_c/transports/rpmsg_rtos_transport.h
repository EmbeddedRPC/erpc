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

#ifndef _EMBEDDED_RPC__RPMSG_RTOS_TRANSPORT_H_
#define _EMBEDDED_RPC__RPMSG_RTOS_TRANSPORT_H_

#include "transport.h"
#include "message_buffer.h"

extern "C" {
#include "rpmsg.h"
#include "rpmsg_rtos.h"
#include "rpmsg.h"
}

/*!
 * @addtogroup rpmsg_rtos_transport
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

enum
{
    kRpmsgMessageBufferSize = RPMSG_BUFFER_SIZE,
};

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc
{
/*!
 * @brief Transport that uses RPMsg RTOS API for interprocessor messaging.
 *
 * @ingroup rpmsg_rtos_transport
 */
class RPMsgRTOSTransport : public Transport
{
public:
    /*!
     * @brief Constructor.
     *
     * This function initializes object attributes.
     */
    RPMsgRTOSTransport();

    /*!
     * @brief RPMsgRTOSTransport destructor
     */
    virtual ~RPMsgRTOSTransport();

    /*!
     * @brief This function call rpmsg rtos init function.
     *
     * @param[in] dev_id Device id number.
     * @param[in] role Device role number.
     *
     * @retval kErpcStatus_Success When rpmsg init function was executed successfully.
     * @retval kErpcStatus_InitFailed When rpmsg init function wasn't executed successfully.
     */
    virtual erpc_status_t init(int dev_id, int role);

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
    virtual erpc_status_t receive(MessageBuffer *message);

    /*!
     * @brief Function to send prepared message.
     *
     * @param[in] message Pass message buffer to send.
     *
     * @retval kErpcStatus_SendFailed Failed to send message buffer.
     * @retval kErpcStatus_Success Successfully sent all data.
     */
    virtual erpc_status_t send(const MessageBuffer *message);

protected:
    /* Remote device */
    static struct remote_device *m_rdev;        /*!< Device which represent the second core. */
    static struct rpmsg_channel *m_app_rp_chnl; /*!< Represent connection between two device (two cores). */
};

class RPMsgMessageBufferFactory : public MessageBufferFactory
{
public:
    /*!
     * @brief Constructor.
     */
    RPMsgMessageBufferFactory() {}
    /*!
     * @brief RPMsgMessageBufferFactory destructor
     */
    virtual ~RPMsgMessageBufferFactory() {}
    /*!
     * @brief This function create message buffer used for communication between devices.
     */
    virtual MessageBuffer create();
    /*!
     * @brief This function dispose message buffer used for communication between devices.
     */
    virtual void dispose(MessageBuffer *buf);
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__RPMSG_RTOS_TRANSPORT_H_
