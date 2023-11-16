/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__UART_TRANSPORT_H_
#define _EMBEDDED_RPC__UART_TRANSPORT_H_

#include "erpc_config_internal.h"
#if !ERPC_THREADS_IS(NONE)
#include "erpc_threading.h"
#endif
#include "erpc_framed_transport.hpp"

extern "C"
{
#include <stdlib.h>
#include <zephyr/drivers/uart.h>
}

/*!
 * @addtogroup uart_transport
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc
{
    /*!
     * @brief Very basic transport to send/receive messages via UART.
     *
     * @ingroup uart_transport
     */
    class UartTransport : public FramedTransport
    {
    public:
        /*!
         * @brief Constructor.
         *
         * @param[in] dev Zephyr uart device.
         */
        UartTransport(struct device *dev);

        /*!
         * @brief Destructor.
         */
        virtual ~UartTransport(void);

        /*!
         * @brief Initialize Zephyr UART peripheral configuration structure with values specified in UartTransport
         * constructor.
         *
         * @retval kErpcStatus_InitFailed When UART init function failed.
         * @retval kErpcStatus_Success When UART init function was executed successfully.
         */
        virtual erpc_status_t init(void);

        /*!
         * @brief Function called from ARM_USART_SignalEvent when ARM_USART_EVENT_RECEIVE_COMPLETE event is asserted
         *
         * Unblocks the receive function.
         */
        void rx_cb(void);

        /*!
         * @brief Function called from ARM_USART_SignalEvent when ARM_USART_EVENT_SEND_COMPLETE event is asserted
         *
         * Unblocks the send function.
         */
        void tx_cb(void);

    protected:
        struct device *m_dev; /*!< Access structure of the UART device */
#if !ERPC_THREADS_IS(NONE)
        Semaphore m_rxSemaphore; /*!< Semaphore used by RTOS to block task until the receiving is not complete */
        Semaphore m_txSemaphore; /*!< Semaphore used by RTOS to block task until the sending is not complete */
#endif
    private:
        /*!
         * @brief Receive data from UART peripheral.
         *
         * @param[inout] data Preallocated buffer for receiving data.
         * @param[in] size Size of data to read.
         *
         * @retval kErpcStatus_ReceiveFailed UART failed to receive data.
         * @retval kErpcStatus_Success Successfully received all data.
         */
        virtual erpc_status_t underlyingReceive(uint8_t *data, uint32_t size);

        /*!
         * @brief Write data to UART peripheral.
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

#endif // _EMBEDDED_RPC__UART_TRANSPORT_H_
