/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__SPIDEV_MASTER_TRANSPORT_H_
#define _EMBEDDED_RPC__SPIDEV_MASTER_TRANSPORT_H_

#include "erpc_framed_transport.hpp"

/*!
 * @addtogroup spidev_master_transport
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc {
/*!
 * @brief Very basic transport to send/receive messages via SPIdev.
 *
 * @ingroup spidev_master_transport
 */
class SpidevMasterTransport : public FramedTransport
{
public:
    /*!
     * @brief Constructor.
     *
     * @param[in] spidev SPI device name.
     * @param[in] speed_Hz SPI clock speed in Hz.
     */
    SpidevMasterTransport(const char *spidev, uint32_t speed_Hz);

    /*!
     * @brief Destructor.
     */
    virtual ~SpidevMasterTransport(void);

    /*!
     * @brief Initialize SPI peripheral.
     *
     * @return Status of init function.
     */
    erpc_status_t init(void);

protected:
    int m_spidevHandle;   /*!< SPI handle id. */
    const char *m_spidev; /*!< SPI device name. */
    uint32_t m_speed_Hz;  /*!< SPI clock speed in Hz. */

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
    virtual erpc_status_t underlyingReceive(uint8_t *data, uint32_t size) override;

    /*!
     * @brief Write data to SPI peripheral.
     *
     * @param[in] data Buffer to send.
     * @param[in] size Size of data to send.
     *
     * @retval kErpcStatus_SendFailed SPI failed to send data.
     * @retval kErpcStatus_Success Successfully sent all data.
     */
    virtual erpc_status_t underlyingSend(const uint8_t *data, uint32_t size) override;
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__SPIDEV_MASTER_TRANSPORT_H_
