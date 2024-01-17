/*
 * Copyright 2023 NXP
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC_ZEPHYR_MBOX_TRANSPORT_H_
#define _EMBEDDED_RPC_ZEPHYR_MBOX_TRANSPORT_H_

#include "erpc_config_internal.h"
#if !ERPC_THREADS_IS(NONE)
#include "erpc_threading.h"
#endif
#include "erpc_transport.hpp"

extern "C" {
#include <stdlib.h>
#include <zephyr/drivers/mbox.h>
#include <zephyr/sys/ring_buffer.h>
}

#define MBOX_BUFFER_SIZE ERPC_DEFAULT_BUFFER_SIZE

/*!
 * @addtogroup mbox_transport
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc {
/*!
 * @brief Transport to send/receive messages via MBOX.
 *
 * @ingroup mbox_transport
 */
class MBOXTransport : public Transport
{
public:
    /*!
     * @brief Constructor.
     *
     * @param[in] dev Zephyr MBOX device.
     * @param[in] tx_channel Zephyr MBOX tx channel.
     * @param[in] rx_channel Zephyr MBOX rx channel.
     */
    MBOXTransport(struct device *dev, struct mbox_channel *tx_channel, struct mbox_channel *rx_channel);

    /*!
     * @brief Destructor.
     */
    virtual ~MBOXTransport(void);

    /*!
     * @brief Initialize Zephyr MBOX peripheral configuration structure with values specified in MBOXTransport
     * constructor.
     *
     * @retval kErpcStatus_InitFailed When MBOC init function failed.
     * @retval kErpcStatus_Success When MBOX init function was executed successfully.
     */
    virtual erpc_status_t init(void);

    /*!
     * @brief Start receiving data and stores it to passed message buffer
     *
     * Initialize receiving of message, it is blocking until doesn't receive complete message.
     *
     * @param[in] message Message buffer, which will be filled by incoming message.
     *
     * @return kErpcStatus_Success
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
     * @brief Function to check if is new message to receive.
     *
     * This function should be called before blocking function receive() to avoid waiting for new message.
     *
     * @return True if exist new message, else false.
     */
    virtual bool hasMessage(void) override;

    /*!
     * @brief Function called from ISR
     *
     * Unblocks the receive function.
     */
    void rx_cb(struct mbox_msg *data);

protected:
    struct device *m_dev; /*!< Access structure of the MBOX device */
    struct mbox_channel *m_tx_channel;
    struct mbox_channel *m_rx_channel;

    
    volatile bool m_isTransferReceiveCompleted = false;
    volatile uint32_t m_transferReceiveRequireBytes = 0;

    /*!
     * @brief Function waits for given amount of bytes to be in ring buffer.
     * 
     * @param[in] numOfBytes Required number of bytes in ring buffer
     */
    void waitForBytes(uint32_t numOfBytes);

#if !ERPC_THREADS_IS(NONE)
    Semaphore m_rxSemaphore; /*!< Semaphore used by RTOS to block task until the receiving is not complete */
    Semaphore m_txSemaphore; /*!< Semaphore used by RTOS to block task until the sending is not complete */

    Mutex m_sendLock;    /*!< Mutex protecting send. */
    Mutex m_receiveLock; /*!< Mutex protecting receive. */
#endif
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC_ZEPHYR_MBOX_TRANSPORT_H_
