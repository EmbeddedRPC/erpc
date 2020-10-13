/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__DSPI_SLAVE_TRANSPORT_H_
#define _EMBEDDED_RPC__DSPI_SLAVE_TRANSPORT_H_

#include "erpc_framed_transport.h"

#include "fsl_dspi.h"

#include <stdlib.h>

/*!
 * @addtogroup dspi_slave_transport
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc {
/*!
 * @brief Very basic transport to send/receive messages via DSPI.
 *
 * @ingroup dspi_slave_transport
 */
class DspiSlaveTransport : public FramedTransport
{
public:
    /*!
     * @brief Constructor.
     *
     * @param[in] spiBaseAddr DSPI peripheral base address.
     * @param[in] baudRate Baudrate.
     * @param[in] srcClock_Hz Source clock.
     */
    DspiSlaveTransport(SPI_Type *spiBaseAddr, uint32_t baudRate, uint32_t srcClock_Hz);

    /*!
     * @brief Destructor.
     */
    virtual ~DspiSlaveTransport(void);

    /*!
     * @brief Initialize DSPI peripheral configuration structure with values specified in DspiTransport constructor.
     *
     * @retval kErpcStatus_Success Always returns success status.
     */
    virtual erpc_status_t init(void);

protected:
    SPI_Type *m_spiBaseAddr; /*!< Base address of DSPI peripheral used in this transport layer */
    uint32_t m_baudRate;     /*!< Baud rate of DSPI peripheral used in this transport layer */
    uint32_t m_srcClock_Hz;  /*!< Source clock of DSPI peripheral used in this transport layer */
    bool m_isInited;         /*!< the SPI peripheral init status flag */

private:
    /*!
     * @brief Receive data from DSPI peripheral.
     *
     * @param[inout] data Preallocated buffer for receiving data.
     * @param[in] size Size of data to read.
     *
     * @retval kErpcStatus_ReceiveFailed DSPI failed to receive data.
     * @retval kErpcStatus_Success Successfully received all data.
     */
    virtual erpc_status_t underlyingReceive(uint8_t *data, uint32_t size);

    /*!
     * @brief Write data to DSPI peripheral.
     *
     * @param[in] data Buffer to send.
     * @param[in] size Size of data to send.
     *
     * @retval kErpcStatus_SendFailed DSPI failed to send data.
     * @retval kErpcStatus_Success Successfully sent all data.
     */
    virtual erpc_status_t underlyingSend(const uint8_t *data, uint32_t size);
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__DSPI_SLAVE_TRANSPORT_H_
