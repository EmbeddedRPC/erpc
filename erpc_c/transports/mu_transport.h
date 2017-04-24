/*
 * Copyright 2017 NXP
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

#ifndef _EMBEDDED_RPC__MU_TRANSPORT_H_
#define _EMBEDDED_RPC__MU_TRANSPORT_H_

#include "erpc_config_internal.h"
#include "erpc_threading.h"
#include "fsl_device_registers.h"
#include "fsl_mu.h"
#include "message_buffer.h"
#include "transport.h"

/*!
 * @addtogroup mu_transport
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////
#define MU_REG_COUNT (MU_RR_COUNT) /*!< Count of MU tx/rx registers to be used by this transport layer */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc {
/*!
 * @brief Transport that uses Messaging Unit (MU) for interprocessor messaging.
 *
 * This transport layer use the Messaging Unit (MU), which enables two processors within
 * the SoC to communicate and coordinate by passing messages (e.g. data, status and control)
 * through the MU interface.
 * The MU has four transmit and four receive registers used for communication between cores,
 * the transport layer use count of registers defined by macro MU_REG_COUNT (default setting
 * is use all MU registers).
 * The communication uses the interrupt from the MU, to be possible use this transport
 * layer on different platform must be specified this macros:
 *      MU_IRQ_HANDLER - name of the MU irq handler
 *      MU_IRQ - name of MU irq to be possible enable it by passing it as argument to function EnableIRQ()
 *      MU_IRQ_PRIORITY - number of priority passed as argument to function NVIC_SetPriority()
 *
 * @ingroup mu_transport
 */

class MUTransport : public Transport
{
public:
    /*!
     * @brief Constructor of MU transport
     *
     * This function initializes object attributes.
     */
    MUTransport();

    /*!
     * @brief Destructor of MU transport
     */
    virtual ~MUTransport();

    /*!
     * @brief Initialization of MU transport layer
     *
     * Initialize MU peripheral and enable MU interrupt in NVIC.
     *
     * @param[in] Base address of MU peripheral
     *
     * @retval kErpcStatus_Success When init function was executed successfully.
     * @retval kErpcStatus_InitFailed When init function wasn't executed successfully.
     */
    virtual erpc_status_t init(MU_Type *muBase);

    /*!
     * @brief Start receiving data and stores it to passed message buffer
     *
     * Initialize receiving of message, it is blocking until doesn't receive complete message.
     *
     * @param[in] message Message buffer, which will be filled by incoming message.
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
     * @brief Function to check if is new message to receive.
     *
     * This function should be called before blocking function receive() to avoid waiting for new message.
     *
     * @return True if exist new message, else false.
     */
    virtual bool hasMessage() { return m_newMessage; }

    /*!
     * @brief Callback function called from MU IRQ
     *
     * This function reads status flags of MU and base on the evnet which cause the irq
     * calls function rx_cb() to handle rx full irq or tx_cb() to handle tx empty irq
     * for the particular instance of the MUTransport
     */
    static void mu_irq_callback();

protected:
    /*!
     * @brief Function called from MU IRQ when the MU RX full flag is asserted and the IRQ is enabled
     *
     * When is this function invoked prior receive function (bare metal case) the m_newMessage
     * flag is set and the MU rx full interrupt is disabled.
     * When is this function invoked after receive function the data from MU registers
     * is copied to this receiving buffer passed in receive function.
     * When is received whole message unblock the receive function.
     */
    void rx_cb();

    /*!
     * @brief Function called from MU IRQ when the MU TX empty flag is asserted and the IRQ is enabled
     *
     * Write data from the buffer passed in send function to MU registers.
     * When is sent whole message unblock the send function.
     */
    void tx_cb();

    volatile bool m_newMessage; /*!< Flag used in function hasMessage() to inform server by polling function that message is ready for receiving */

    uint32_t m_rxMsgSize;          /*!< Size of received message - count of bytes to must be received to compltete currently received message */
    uint32_t m_rxCntBytes;         /*!< Count of currently received bytes of message */
    uint32_t *volatile m_rxBuffer; /*!< Pointer to buffer to which is copied data from MU registers during receiving */

    uint32_t m_txMsgSize;          /*!< Size of transmitted message - count of bytes to must be transmitted to send compltete message */
    uint32_t m_txCntBytes;         /*!< Count of currently received bytes of message */
    uint32_t *volatile m_txBuffer; /*!< Pointer to buffer from which is copied data to MU registers during sending */

#if ERPC_THREADS
    Semaphore m_rxSemaphore; /*!< Semaphore used by RTOS to block task until the receiving is not complete */
    Semaphore m_txSemaphore; /*!< Semaphore used by RTOS to block task until the sending is not complete */

    Mutex m_sendLock;    /*!< Mutex protecting send. */
    Mutex m_receiveLock; /*!< Mutex protecting receive. */
#endif

    MU_Type *m_muBase; /*!< Base address of MU peripheral */
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__MU_TRANSPORT_H_
