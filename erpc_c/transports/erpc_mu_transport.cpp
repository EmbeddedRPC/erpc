/*
 * Copyright 2017-2023 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_mu_transport.hpp"

#include "erpc_config_internal.h"

extern "C" {
#include "board.h"
}

using namespace erpc;

/* IMPLEMNENTATION COMMENTS */
/*
   Implementation for baremetal (#if !ERPC_THREADS_IS(NONE) conditional section)
   is nonblocking, the code flow is following: enable interrupt, wait for interrupt, once interrupt appears disable the
   interrupt and set hasMessage==true, recv start, enable interrupt, get data, receive end (keep interrupt enabled for
   the same reason as the first time).

   Implementation for an RTOS (FREERTOS) is blocking, the code flow is following: enable interrupt on recv start,
   get data once ready, disable interrupt on receive end.
*/

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////
static MUTransport *s_mu_instance = NULL;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////
#if ERPC_TRANSPORT_MU_USE_MCMGR
void MUTransport::mu_tx_empty_irq_callback(void)
{
    MUTransport *transport = s_mu_instance;
    if ((transport != NULL) && MU_IS_TX_EMPTY_FLAG_SET)
    {
        transport->tx_cb();
    }
}

void MUTransport::mu_rx_full_irq_callback(void)
{
    MUTransport *transport = s_mu_instance;
    if ((transport != NULL) && MU_IS_RX_FULL_FLAG_SET)
    {
        transport->rx_cb();
    }
}
#else
void MUTransport::mu_irq_callback(void)
{
    MUTransport *transport = s_mu_instance;
    uint32_t flags;

    flags = MU_GetStatusFlags(transport->m_muBase);

    // RECEIVING - rx full flag and rx full irq enabled
    if ((flags & MU_SR_RX_MASK) && MU_IS_RX_FULL_FLAG_SET)
    {
        transport->rx_cb();
    }

    // TRANSMITTING - tx empty flag and tx empty irq enabled
    if ((flags & MU_SR_TX_MASK) && MU_IS_TX_EMPTY_FLAG_SET)
    {
        transport->tx_cb();
    }
}
#endif /* ERPC_TRANSPORT_MU_USE_MCMGR */

MUTransport::MUTransport(void)
: Transport()
, m_newMessage(false)
, m_rxMsgSize(0)
, m_rxCntBytes(0)
, m_rxBuffer(NULL)
, m_txMsgSize(0)
, m_txCntBytes(0)
, m_txBuffer(NULL)
#if !ERPC_THREADS_IS(NONE)
, m_rxSemaphore()
, m_txSemaphore()
, m_sendLock()
, m_receiveLock()
#endif
, m_muBase(NULL)
{
    s_mu_instance = this;
}

MUTransport::~MUTransport(void) {}

erpc_status_t MUTransport::init(MU_Type *muBase)
{
    m_muBase = muBase;
#if !ERPC_TRANSPORT_MU_USE_MCMGR
    MU_Init(muBase);
#endif

#if !ERPC_THREADS
    // enabling the MU rx full irq is necessary only for BM app
    MU_EnableInterrupts(m_muBase, MU_RX_INTR_MASK);
#endif

    NVIC_SetPriority(MU_IRQ, MU_IRQ_PRIORITY);
    (void)EnableIRQ(MU_IRQ);

    return kErpcStatus_Success;
}

void MUTransport::rx_cb(void)
{
    uint32_t i = 0;

    if (m_rxBuffer == NULL)
    {
        // the receive function has not been called yet
        // disable MU rx full interrupt
        MU_DisableInterrupts(m_muBase, MU_RX_INTR_MASK);
        m_newMessage = true;
    }
    else
    {
        // read data from the MU rx registers
        if (m_rxMsgSize == 0U)
        {
            m_rxMsgSize = MU_ReceiveMsgNonBlocking(m_muBase, i++);
        }

        for (; i < MU_REG_COUNT; i++)
        {
            if (m_rxCntBytes < m_rxMsgSize)
            {
                m_rxBuffer[m_rxCntBytes >> 2] = MU_ReceiveMsgNonBlocking(m_muBase, i);
                m_rxCntBytes += 4U;
            }
            else
            {
                // read MU rx reg to clear the rx full flag
                (void)MU_ReceiveMsgNonBlocking(m_muBase, i);
            }
        }

        // message is complete, unblock caller of the receive function
        if (m_rxCntBytes >= m_rxMsgSize)
        {
            m_rxBuffer = NULL;
#if !ERPC_THREADS_IS(NONE)
            // disable MU rx full interrupt in rtos-based blocking implementation
            MU_DisableInterrupts(m_muBase, MU_RX_INTR_MASK);
            m_rxSemaphore.putFromISR();
#endif
        }
    }
}

void MUTransport::tx_cb(void)
{
    // fill MU tx regs
    uint32_t i = 0;
    uint32_t tx;

    for (i = 0; i < MU_REG_COUNT; i++)
    {
        // prepare uint32_t and write it to next MU tx reg
        tx = 0;
        if (m_txCntBytes < m_txMsgSize)
        {
            tx = m_txBuffer[m_txCntBytes >> 2];
        }
        MU_SendMsg(m_muBase, i, tx);
        m_txCntBytes += 4U;
    }

    // if the message was sent whole, disable the MU tx empty irq
    if (m_txCntBytes >= m_txMsgSize)
    {
        // disable MU tx empty irq from the last tx reg
        MU_DisableInterrupts(m_muBase, MU_TX_INTR_MASK);

        // unblock caller of the send function
        m_txBuffer = NULL;
#if !ERPC_THREADS_IS(NONE)
        m_txSemaphore.putFromISR();
#endif
    }
}

erpc_status_t MUTransport::receive(MessageBuffer *message)
{
    erpc_status_t status;

    if (message == NULL)
    {
        status = kErpcStatus_ReceiveFailed;
    }
    else
    {
#if !ERPC_THREADS_IS(NONE)
        Mutex::Guard lock(m_receiveLock);
#endif

        m_rxMsgSize = 0;
        m_rxCntBytes = 0;
        m_rxBuffer = reinterpret_cast<uint32_t *>(message->get());

        // enable the MU rx full irq
        MU_EnableInterrupts(m_muBase, MU_RX_INTR_MASK);

// wait until the receiving is not complete
#if !ERPC_THREADS_IS(NONE)
        (void)m_rxSemaphore.get();
#else
        while (m_rxBuffer != NULL)
        {
        }
#endif

        message->setUsed((uint16_t)m_rxMsgSize);
        m_newMessage = false;
        status = kErpcStatus_Success;
    }

    return status;
}

erpc_status_t MUTransport::send(MessageBuffer *message)
{
    erpc_status_t status;
    uint8_t i;
    uint32_t tx;

    if (message == NULL)
    {
        status = kErpcStatus_SendFailed;
    }
    else
    {
#if !ERPC_THREADS_IS(NONE)
        Mutex::Guard lock(m_sendLock);
#endif

        m_txMsgSize = message->getUsed();
        m_txCntBytes = 0;
        m_txBuffer = reinterpret_cast<uint32_t *>(message->get());

        MU_SendMsg(m_muBase, 0, m_txMsgSize);

        // write to next MU tx registers
        for (i = 1; i < MU_REG_COUNT; i++)
        {
            // prepare next uint32_t and write it to next MU tx reg
            tx = 0;
            if (m_txCntBytes < m_txMsgSize)
            {
                tx = m_txBuffer[m_txCntBytes >> 2];
            }
            MU_SendMsg(m_muBase, i, tx);
            m_txCntBytes += 4U;
        }

        // if the message wasn't sent whole, enable the MU tx empty irq
        if (m_txCntBytes < m_txMsgSize)
        {
            // enable MU tx empty irq from the last mu tx reg
            MU_EnableInterrupts(m_muBase, MU_TX_INTR_MASK);
// wait until the sending is not complete
#if !ERPC_THREADS_IS(NONE)
            (void)m_txSemaphore.get();
#else
            while (m_txBuffer != NULL)
            {
            }
#endif
        }
        status = kErpcStatus_Success;
    }

    return status;
}

extern "C" {

#if ERPC_TRANSPORT_MU_USE_MCMGR
/*!
 * @brief Messaging Unit TxEmptyFlag ISR handler
 *
 * This function overloads the weak handler defined in MCMGR MU_interrupts[] MU ISR table
 */
void MU_TxEmptyFlagISRCallback(void)
{
    MUTransport::mu_tx_empty_irq_callback();
}

/*!
 * @brief Messaging Unit RxFullFlag ISR handler
 *
 * This function overloads the weak handler defined in MCMGR MU_interrupts[] MU ISR table
 */
void MU_RxFullFlagISRCallback(void)
{
    MUTransport::mu_rx_full_irq_callback();
}
#else
int MU_IRQ_HANDLER(void)
{
    MUTransport::mu_irq_callback();
    return 0;
}
#endif /* ERPC_TRANSPORT_MU_USE_MCMGR */
}
