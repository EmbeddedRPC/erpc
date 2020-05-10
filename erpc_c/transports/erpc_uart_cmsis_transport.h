/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__UART_TRANSPORT_H_
#define _EMBEDDED_RPC__UART_TRANSPORT_H_

#include "erpc_framed_transport.h"
#include "Driver_USART.h"
#include <stdlib.h>

/*!
 * @addtogroup uart_transport
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc {
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
     * @param[in] uartDrv Cmsis uart.
     */
    UartTransport(ARM_DRIVER_USART *uartDrv);

    /*!
     * @brief Destructor.
     */
    virtual ~UartTransport(void);

    /*!
     * @brief Initialize CMSIS UART peripheral configuration structure with values specified in UartTransport
     * constructor.
     *
     * @retval kErpcStatus_InitFailed When UART init function failed.
     * @retval kErpcStatus_Success When UART init function was executed successfully.
     */
    virtual erpc_status_t init(void);

protected:
    ARM_DRIVER_USART *m_uartDrv; /*!< Access structure of the USART Driver */

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
