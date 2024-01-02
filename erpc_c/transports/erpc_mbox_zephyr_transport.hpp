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
#include "erpc_framed_transport.hpp"

extern "C" {
#include <stdlib.h>
}

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
class MBOXTransport : public FramedTransport
{
public:
    /*!
     * @brief Constructor.
     *
     * @param[in] dev Zephyr MBOX device.
     */
    MBOXTransport(struct device *dev);

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
     * @brief Function called from ARM_USART_SignalEvent when ARM_USART_EVENT_RECEIVE_COMPLETE event is asserted
     *
     * Unblocks the receive function.
     */
    void rx_cb(void);

protected:
    struct device *m_dev; /*!< Access structure of the MBOX device */
    struct mbox_channel m_tx_channel;
	struct mbox_channel m_rx_channel;
#if !ERPC_THREADS_IS(NONE)
    Semaphore m_rxSemaphore; /*!< Semaphore used by RTOS to block task until the receiving is not complete */
    Semaphore m_txSemaphore; /*!< Semaphore used by RTOS to block task until the sending is not complete */
#endif
private:
    /*!
     * @brief Receive data from MBOX channel.
     *
     * @param[inout] data Preallocated buffer for receiving data.
     * @param[in] size Size of data to read.
     *
     * @retval kErpcStatus_ReceiveFailed MBOX failed to receive data.
     * @retval kErpcStatus_Success Successfully received all data.
     */
    virtual erpc_status_t underlyingReceive(uint8_t *data, uint32_t size);

    /*!
     * @brief Write data to MBOX channel.
     *
     * @param[in] data Buffer to send.
     * @param[in] size Size of data to send.
     *
     * @retval kErpcStatus_Success Always returns success status.
     */
    virtual erpc_status_t underlyingSend(const uint8_t *data, uint32_t size);
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC_ZEPHYR_MBOX_TRANSPORT_H_
