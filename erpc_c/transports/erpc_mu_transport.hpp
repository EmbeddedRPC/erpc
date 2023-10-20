/*
 * Copyright 2017-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__MU_TRANSPORT_H_
#define _EMBEDDED_RPC__MU_TRANSPORT_H_

#include "erpc_config_internal.h"
#if !ERPC_THREADS_IS(NONE)
#include "erpc_threading.h"
#endif
#include "erpc_message_buffer.hpp"
#include "erpc_transport.hpp"

extern "C" {
#include "fsl_device_registers.h"
#include "fsl_mu.h"
}

/*!
 * @addtogroup mu_transport
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////
#if ERPC_TRANSPORT_MU_USE_MCMGR
/*!< Count of MU tx/rx registers to be used by this transport layer.
     Keep one MU channel for MCMGR operations. */
#define MU_REG_COUNT (MU_RR_COUNT - 1U)
#define MU_LAST_REG_IDX 2

#define MU_RX_Interrupt_Handler(x) MU_RX_Interrupt(x)
#define MU_RX_Interrupt(number) MU_Rx##number##FullFlagISR
#define MU_RxFullFlagISRCallback MU_RX_Interrupt_Handler(MU_LAST_REG_IDX)

#define MU_TX_Interrupt_Handler(x) MU_TX_Interrupt(x)
#define MU_TX_Interrupt(number) MU_Tx##number##EmptyFlagISR
#define MU_TxEmptyFlagISRCallback MU_TX_Interrupt_Handler(MU_LAST_REG_IDX)

#else
#define MU_REG_COUNT (MU_RR_COUNT) /*!< Count of MU tx/rx registers to be used by this transport layer */
#endif                             /* ERPC_TRANSPORT_MU_USE_MCMGR */

#if (defined(MIMXRT1187_cm7_SERIES) || defined(MIMXRT1187_cm33_SERIES) || defined(MIMXRT1189_cm7_SERIES) || \
     defined(MIMXRT1189_cm33_SERIES))
#define MU_TX_SHIFT (1UL << (MU_REG_COUNT - 1U))
#define MU_RX_SHIFT (1UL << (MU_REG_COUNT - 1U))
#define MU_RX_INTR_MASK (MU_RX_INTR(MU_RX_SHIFT))
#define MU_TX_INTR_MASK (MU_TX_INTR(MU_TX_SHIFT))
#define MU_IS_TX_EMPTY_FLAG_SET (0U != (transport->m_muBase->TCR & MU_TX_SHIFT))
#define MU_IS_RX_FULL_FLAG_SET (0U != (transport->m_muBase->RCR & MU_RX_SHIFT))
#define MU_SR_TX_MASK (1UL << (20U + (MU_REG_COUNT - 1U)))
#define MU_SR_RX_MASK (1UL << (24U + (MU_REG_COUNT - 1U)))
#else
#define MU_TX_SHIFT (1UL << (MU_CR_TIEn_SHIFT + MU_TR_COUNT - MU_REG_COUNT))
#define MU_RX_SHIFT (1UL << (MU_CR_RIEn_SHIFT + MU_RR_COUNT - MU_REG_COUNT))
#define MU_RX_INTR_MASK (MU_RX_SHIFT)
#define MU_TX_INTR_MASK (MU_TX_SHIFT)
#define MU_IS_TX_EMPTY_FLAG_SET (0U != (transport->m_muBase->CR & MU_TX_SHIFT))
#define MU_IS_RX_FULL_FLAG_SET (0U != (transport->m_muBase->CR & MU_RX_SHIFT))
#define MU_SR_TX_MASK (1UL << (MU_SR_TEn_SHIFT + MU_TR_COUNT - MU_REG_COUNT))
#define MU_SR_RX_MASK (1UL << (MU_SR_RFn_SHIFT + MU_RR_COUNT - MU_REG_COUNT))
#endif
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
    MUTransport(void);

    /*!
     * @brief Destructor of MU transport
     */
    virtual ~MUTransport(void);

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
    virtual bool hasMessage(void) { return m_newMessage; }

#if ERPC_TRANSPORT_MU_USE_MCMGR
    /*!
     * @brief Callback function called from MU IRQ when TxEmptyFlag is set
     *
     * This function calls tx_cb() to handle tx empty irq for the particular instance of the MUTransport.
     * MU interrupts are managed by the MCMGR component and the mu_transport overloads the weak handler
     * defined in MCMGR MU ISR table.
     */

    static void mu_tx_empty_irq_callback(void);
    /*!
     * @brief Callback function called from MU IRQ when RxFullFlag is set
     *
     * This function calls rx_cb() to handle rx full irq for the particular instance of the MUTransport.
     * MU interrupts are managed by the MCMGR component and the mu_transport overloads the weak handler
     * defined in MCMGR MU ISR table.
     */
    static void mu_rx_full_irq_callback(void);
#else
    /*!
     * @brief Callback function called from MU IRQ
     *
     * This function reads status flags of MU and base on the evnet which cause the irq
     * calls function rx_cb() to handle rx full irq or tx_cb() to handle tx empty irq
     * for the particular instance of the MUTransport
     */
    static void mu_irq_callback(void);
#endif /* ERPC_TRANSPORT_MU_USE_MCMGR */

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
    void rx_cb(void);

    /*!
     * @brief Function called from MU IRQ when the MU TX empty flag is asserted and the IRQ is enabled
     *
     * Write data from the buffer passed in send function to MU registers.
     * When is sent whole message unblock the send function.
     */
    void tx_cb(void);

    volatile bool m_newMessage;    /*!< Flag used in function hasMessage() to inform server by polling function that
                                      message is ready for receiving */
    volatile uint32_t m_rxMsgSize; /*!< Size of received message - count of bytes to must be received to complete
                             currently received message */
    uint32_t m_rxCntBytes;         /*!< Count of currently received bytes of message */
    uint32_t *volatile m_rxBuffer; /*!< Pointer to buffer to which is copied data from MU registers during receiving */
    volatile uint32_t m_txMsgSize; /*!< Size of transmitted message - count of bytes to must be transmitted to send
                             complete message */
    uint32_t m_txCntBytes;         /*!< Count of currently received bytes of message */
    uint32_t *volatile m_txBuffer; /*!< Pointer to buffer from which is copied data to MU registers during sending */

#if !ERPC_THREADS_IS(NONE)
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
