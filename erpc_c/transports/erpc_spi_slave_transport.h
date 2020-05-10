/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__SPI_SLAVE_TRANSPORT_H_
#define _EMBEDDED_RPC__SPI_SLAVE_TRANSPORT_H_

#include "erpc_framed_transport.h"
#include "fsl_gpio.h"
#include "fsl_spi.h"
#include <stdlib.h>

/*!
 * @addtogroup spi_slave_transport
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
 * @ingroup spi_slave_transport
 */
class SpiSlaveTransport : public FramedTransport
{
public:
    /*!
     * @brief Constructor.
     *
     * @param[in] spiBaseAddr DSPI peripheral base address.
     * @param[in] baudRate Baudrate.
     * @param[in] srcClock_Hz Source clock.
     */
    SpiSlaveTransport(SPI_Type *spiBaseAddr, uint32_t baudRate, uint32_t srcClock_Hz);

    /*!
     * @brief Destructor.
     */
    virtual ~SpiSlaveTransport(void);

    /*!
     * @brief Initialize SPI peripheral configuration structure with values specified in SpiTransport constructor.
     *
     * @retval kErpcStatus_Success Always returns success status.
     */
    virtual erpc_status_t init(void);

protected:
    SPI_Type *m_spiBaseAddr; /*!< Base address of SPI peripheral used in this transport layer */
    uint32_t m_baudRate;     /*!< Baud rate of SPI peripheral used in this transport layer */
    uint32_t m_srcClock_Hz;  /*!< Source clock of SPI peripheral used in this transport layer */
    bool m_isInited;         /*!< the SPI peripheral init status flag */

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

#endif // _EMBEDDED_RPC__SPI_SLAVE_TRANSPORT_H_
