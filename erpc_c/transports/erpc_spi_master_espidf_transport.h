/*
 * Copyright (c) 2020 Sierra Wireless
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__SPI_MASTER_TRANSPORT_H_
#define _EMBEDDED_RPC__SPI_MASTER_TRANSPORT_H_

#include "erpc_framed_transport.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include <stdbool.h>
#include <stdlib.h>

/*!
 * @addtogroup spi_master_transport
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc {
/*!
 * @brief Very basic transport to send/receive messages via SPI.
 *
 * @ingroup spi_master_transport
 */
class SpiMasterTransport : public FramedTransport
{
public:
    /*!
     * @brief Constructor.
     *
     * @param[in] bus_config a pointer to a spi_bus_config_t or NULL if bus already initialized
     * @param[in] host user host SPI2_HOST or SPI3_HOST (1 is reserved)
     * @param[in] cs_gpio chip select gpio (-1 to disable)
     * @param[in] ready_gpio gpio used for handshake - a positive edge signals the slave is ready (-1 if not used)
     * @param[in] speed bus speed in Hz (0 or -1 for auto)
     
     */
    SpiMasterTransport(spi_bus_config_t *bus_config, spi_host_device_t host = SPI2_HOST, gpio_num_t cs_gpio = GPIO_NUM_NC, 
                       gpio_num_t ready_gpio = GPIO_NUM_NC, int speed = 0);

    /*!
     * @brief Destructor.
     */
    virtual ~SpiMasterTransport(void);

    /*!
     * @brief Initialize SPI peripheral configuration structure with values specified in SpiTransport constructor.
     *
     * @param[in] timeout rx/tx request timeout in ms
     * @retval kErpcStatus_Success Always returns success status.
     */
    virtual erpc_status_t init(int timeout = -1);

protected:
    struct {
        spi_bus_config_t *bus_config;
        spi_device_interface_config_t config;
        spi_host_device_t host;
        spi_device_handle_t handle;
    } m_spi    ;
    SemaphoreHandle_t m_ready_sem;
    gpio_num_t m_ready_gpio;
    TickType_t m_timeout;

private:
    /*!
     * @brief Receive data from SPI peripheral.
     *
     * @param[inout] data Preallocated buffer for receiving data.
     * @param[in] size Size of data to read.
     *
     * @retval kErpcStatus_ReceiveFailed SPI failed to receive data.
     * @retval kErpcStatus_Success Successfully received all data.
     */
    virtual erpc_status_t underlyingReceive(uint8_t *data, uint32_t size);

    /*!
     * @brief Write data to SPI peripheral.
     *
     * @param[in] data Buffer to send.
     * @param[in] size Size of data to send.
     *
     * @retval kErpcStatus_SendFailed SPI failed to send data.
     * @retval kErpcStatus_Success Successfully sent all data.
     */
    virtual erpc_status_t underlyingSend(const uint8_t *data, uint32_t size);
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__SPI_MASTER_TRANSPORT_H_
