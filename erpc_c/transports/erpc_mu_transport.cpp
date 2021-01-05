/*
 * Copyright 2017-2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_mu_transport.h"

#include "erpc_config_internal.h"

#include "board.h"

using namespace erpc;

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
    if ((transport) && (transport->m_muBase->CR & (1U << (MU_CR_TIEn_SHIFT + MU_TR_COUNT - MU_REG_COUNT))))
    {
        transport->tx_cb();
    }
}

void MUTransport::mu_rx_full_irq_callback(void)
{
    MUTransport *transport = s_mu_instance;
    if ((transport) && (transport->m_muBase->CR & (1U << (MU_CR_RIEn_SHIFT + MU_RR_COUNT - MU_REG_COUNT))))
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

    // parse flags of full rx registers
    uint32_t rxFlags = ((flags & MU_SR_RFn_MASK) >> MU_SR_RFn_SHIFT);
    rxFlags = (rxFlags >> (MU_RR_COUNT - MU_REG_COUNT));

    // parse flags of empty tx registers
    uint32_t txFlags = ((flags & MU_SR_TEn_MASK) >> MU_SR_TEn_SHIFT);
    txFlags = (txFlags >> (MU_TR_COUNT - MU_REG_COUNT));

    // RECEIVING - rx full flag and rx full irq enabled
    if ((rxFlags & 0x1) && (transport->m_muBase->CR & (1U << (MU_CR_RIEn_SHIFT + MU_RR_COUNT - MU_REG_COUNT))))
    {
        transport->rx_cb();
    }

    // TRANSMITTING - tx empty flag and tx empty irq enabled
    if ((txFlags & 0x1) && (transport->m_muBase->CR & (1U << (MU_CR_TIEn_SHIFT + MU_TR_COUNT - MU_REG_COUNT))))
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
    MU_EnableInterrupts(m_muBase, (1U << (MU_CR_RIEn_SHIFT + MU_RR_COUNT - MU_REG_COUNT)));
#endif

    NVIC_SetPriority(MU_IRQ, MU_IRQ_PRIORITY);
    EnableIRQ(MU_IRQ);

    return kErpcStatus_Success;
}

void MUTransport::rx_cb(void)
{
    if (m_rxBuffer == NULL)
    {
        // the receive function has not been called yet
        // disable MU rx full interrupt
        MU_DisableInterrupts(m_muBase, (1U << (MU_CR_RIEn_SHIFT + MU_RR_COUNT - MU_REG_COUNT)));
        m_newMessage = true;
        return;
    }

    // read data from the MU rx registers
    uint32_t i = 0;
    if (m_rxMsgSize == 0)
    {
        m_rxMsgSize = MU_ReceiveMsgNonBlocking(m_muBase, i++);
    }

    for (; i < MU_REG_COUNT; i++)
    {
        if (m_rxCntBytes < m_rxMsgSize)
        {
            m_rxBuffer[m_rxCntBytes >> 2] = MU_ReceiveMsgNonBlocking(m_muBase, i);
            m_rxCntBytes += 4;
        }
        else
        {
            // read MU rx reg to clear the rx full flag
            MU_ReceiveMsgNonBlocking(m_muBase, i);
        }
    }

    // message is complete, unblock caller of the receive function
    if (m_rxCntBytes >= m_rxMsgSize)
    {
        m_rxBuffer = NULL;
#if !ERPC_THREADS_IS(NONE)
        MU_DisableInterrupts(m_muBase, (1U << (MU_CR_RIEn_SHIFT + MU_RR_COUNT - MU_REG_COUNT)));
        m_rxSemaphore.putFromISR();
#endif
    }
}

void MUTransport::tx_cb(void)
{
    // fill MU tx regs
    uint32_t i = 0;
    for (i = 0; i < MU_REG_COUNT; i++)
    {
        // prepare uint32_t and write it to next MU tx reg
        uint32_t tx = 0;
        if (m_txCntBytes < m_txMsgSize)
        {
            tx = m_txBuffer[m_txCntBytes >> 2];
        }
        MU_SendMsgNonBlocking(m_muBase, i, tx);
        m_txCntBytes += 4;
    }

    // if the message was sent whole, disable the MU tx empty irq
    if (m_txCntBytes >= m_txMsgSize)
    {
        // disable MU tx empty irq from the last tx reg
        MU_DisableInterrupts(m_muBase, (1U << (MU_CR_TIEn_SHIFT + MU_TR_COUNT - MU_REG_COUNT)));

        // unblock caller of the send function
        m_txBuffer = NULL;
#if !ERPC_THREADS_IS(NONE)
        m_txSemaphore.putFromISR();
#endif
    }
}

erpc_status_t MUTransport::receive(MessageBuffer *message)
{
    if (message == NULL)
    {
        return kErpcStatus_ReceiveFailed;
    }

#if !ERPC_THREADS_IS(NONE)
    Mutex::Guard lock(m_receiveLock);
#endif

    m_rxMsgSize = 0;
    m_rxCntBytes = 0;
    m_rxBuffer = (uint32_t *)message->get();

    // enable the MU rx full irq
    MU_EnableInterrupts(m_muBase, (1U << (MU_CR_RIEn_SHIFT + MU_RR_COUNT - MU_REG_COUNT)));

// wait until the receiving is not complete
#if !ERPC_THREADS_IS(NONE)
    m_rxSemaphore.get();
#else
    while (m_rxBuffer)
    {
    }
#endif

    message->setUsed(m_rxMsgSize);
    m_newMessage = false;

    return kErpcStatus_Success;
}

erpc_status_t MUTransport::send(MessageBuffer *message)
{
    if (message == NULL)
    {
        return kErpcStatus_SendFailed;
    }

#if !ERPC_THREADS_IS(NONE)
    Mutex::Guard lock(m_sendLock);
#endif

    m_txMsgSize = message->getUsed();
    m_txCntBytes = 0;
    m_txBuffer = (uint32_t *)message->get();

    MU_SendMsgNonBlocking(m_muBase, 0, m_txMsgSize);

    // write to next MU tx registers
    uint8_t i = 0;
    for (i = 1; i < MU_REG_COUNT; i++)
    {
        // prepare next uint32_t and write it to next MU tx reg
        uint32_t tx = 0;
        if (m_txCntBytes < m_txMsgSize)
        {
            tx = m_txBuffer[m_txCntBytes >> 2];
        }
        MU_SendMsgNonBlocking(m_muBase, i, tx);
        m_txCntBytes += 4;
    }

    // if the message wasn't sent whole, enable the MU tx empty irq
    if (m_txCntBytes < m_txMsgSize)
    {
        // enable MU tx empty irq from the last mu tx reg
        MU_EnableInterrupts(m_muBase, (1U << (MU_CR_TIEn_SHIFT + MU_TR_COUNT - MU_REG_COUNT)));
// wait until the sending is not complete
#if !ERPC_THREADS_IS(NONE)
        m_txSemaphore.get();
#else
        while (m_txBuffer)
        {
        }
#endif
    }
    return kErpcStatus_Success;
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
