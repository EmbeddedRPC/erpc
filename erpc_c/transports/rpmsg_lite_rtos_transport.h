/*
 * Copyright (c) 2015-2016, Freescale Semiconductor, Inc.
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

#ifndef _EMBEDDED_RPC__RPMSG_LITE_RTOS_TRANSPORT_H_
#define _EMBEDDED_RPC__RPMSG_LITE_RTOS_TRANSPORT_H_

#include "transport.h"
#include "message_buffer.h"
#include "rpmsg_lite.h"
#include "rpmsg_queue.h"

/*!
 * @addtogroup rpmsg_lite_rtos_transport
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
 * @brief Transport that uses RPMsg RTOS API for interprocessor messaging.
 *
 * @ingroup rpmsg_lite_rtos_transport
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
     * @brief This function call RPMsg rtos init function - as RPMsg master
     *
     * @param[in] src_addr Source address.
     * @param[in] dst_addr Destination address.
     * @param[in] role Device role number.
     * @param[in] base_address RPMsg base address in the shared memory
     * @param[in] length RPMsg shared memory region length
     *
     * @retval kErpcStatus_Success When rpmsg init function was executed successfully.
     * @retval kErpcStatus_Fail When rpmsg init function wasn't executed successfully.
     */
    virtual status_t init(unsigned long src_addr, unsigned long dst_addr, void *base_address, unsigned long length);

    /*!
     * @brief This function call RPMsg rtos init function - as RPMsg remote
     *
     * @param[in] dev_id Device id number.
     * @param[in] role Device role number.
     * @param[in] base_address RPMsg base address in the shared memory
     *
     * @retval kErpcStatus_Success When rpmsg init function was executed successfully.
     * @retval kErpcStatus_Fail When rpmsg init function wasn't executed successfully.
     */
    virtual status_t init(unsigned long src_addr, unsigned long dst_addr, void *base_address);

    /*!
     * @brief Store incoming message to message buffer.
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

protected:
    /* Remote device */
    struct remote_device *m_rdev;            /*!< Device which represent the second core. */
    struct rpmsg_channel *m_app_rp_chnl;     /*!< Represent connection between two device (two cores). */
    unsigned long m_dst_addr;                /*!< Destination address used by rpmsg. */
    rpmsg_queue_handle m_rpmsg_queue;        /*!< Handle of RPMsg queue. */
    struct rpmsg_lite_endpoint *m_rpmsg_ept; /*!< Pointer to RPMsg Lite Endpoint structure. */

    static struct rpmsg_lite_instance *s_rpmsg; /*!< Pointer to instance of RPMSG lite. */
    static uint8_t s_initialized;               /*!< Represent information if the rpmsg-lite was initialized. */
};

class RPMsgMessageBufferFactory : public MessageBufferFactory
{
    uint8_t m_freeBufferBitmap;
    uint8_t m_buffers[kInitCountMessageBuffers][kRpmsgMessageBufferSize];

public:
    /*!
     * @brief Constructor.
     */
    RPMsgMessageBufferFactory()
    : m_freeBufferBitmap(0xFF)
    {
    }
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

#endif // _EMBEDDED_RPC__RPMSG_LITE_RTOS_TRANSPORT_H_
