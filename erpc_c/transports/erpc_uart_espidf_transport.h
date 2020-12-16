/*
 * Copyright (c) 2020 Sierra Wireless
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__UART_TRANSPORT_H_
#define _EMBEDDED_RPC__UART_TRANSPORT_H_

#include "erpc_framed_transport.h"
#include "driver/uart.h"
#include "driver/gpio.h"
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
     * @param[in] uart_config an esp-idf uart config structure
     * @param[in] uart_num UART_NUM_1 ... UART_NUM_MAX
     * @param[in] rx_gpio RX gpio
     * @param[in] tx_gpio TX gpio
     * @param[in] rts_gpio RTS gpio (-1 to disable)
     * @param[in] cts_gpio CTS gpio (-1 to disable)
     */
    UartTransport(uart_config_t *uart_config, uart_port_t uart_num = UART_NUM_1,
                 gpio_num_t rx_gpio = GPIO_NUM_NC, gpio_num_t tx_gpio = GPIO_NUM_NC,
                 gpio_num_t rts_gpio = GPIO_NUM_NC, gpio_num_t cts_gpio = GPIO_NUM_NC);

    /*!
     * @brief Destructor.
     */
    virtual ~UartTransport(void);

    /*!
     * @brief Initialize CMSIS UART peripheral configuration structure with values specified in UartTransport
     * constructor.
     *
     * @param[in] timeout timeout in ms for reception (-1 to wait forever)
     * @retval kErpcStatus_InitFailed When UART init function failed.
     * @retval kErpcStatus_Success When UART init function was executed successfully.
     */
    virtual erpc_status_t init(int timeout = -1);

    /*
     * Tells clients if the transport has a message available
     */
    virtual bool hasMessage(void);

protected:
    struct {
        uart_config_t config;
        uart_port_t num;
        gpio_num_t rx_gpio, tx_gpio, rts_gpio, cts_gpio;
        int rx_ring_size, tx_ring_size, queue_size;
        QueueHandle_t *queue;
        int intr_alloc_flags;
    } m_uart;
    TickType_t m_timeout;

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
