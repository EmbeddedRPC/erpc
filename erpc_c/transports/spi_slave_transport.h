/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _EMBEDDED_RPC__SPI_SLAVE_TRANSPORT_H_
#define _EMBEDDED_RPC__SPI_SLAVE_TRANSPORT_H_

#include <stdlib.h>
#include "framed_transport.h"
#include "fsl_spi.h"
#include "fsl_gpio.h"

/*!
 * @addtogroup spi_slave_transport
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc
{
/*!
 * @brief Very basic transport to send/receive messages via SPI.
 *
 * @ingroup spi_slave_transport
 */
class SpiSlaveTransport : public FramedTransport
{
public:
    SpiSlaveTransport(SPI_Type *spiBaseAddr, uint32_t baudRate, uint32_t srcClock_Hz);
    virtual ~SpiSlaveTransport();

    /*!
     * @brief Initialize SPI peripheral configuration structure with values specified in SpiTransport constructor.
     *
     * @retval kErpcStatus_Success Always returns success status.
     */
    virtual erpc_status_t init();

protected:
    SPI_Type *m_spiBaseAddr; /*!< Base address of SPI peripheral used in this transport layer */
    uint32_t m_baudRate;     /*!< Baud rate of SPI peripheral used in this transport layer */
    uint32_t m_srcClock_Hz;  /*!< Source clock of SPI peripheral used in this transport layer */

private:
    /*!
     * @brief Receive data from SPI peripheral.
     *
     * @param[inout] data Preallocated buffer for receiving data.
     * @param[in] data Size of data to read.
     *
     * @retval kErpcStatus_ReceiveFailed SPI failed to receive data.
     * @retval kErpcStatus_Success Successfully received all data.
     */
    virtual erpc_status_t underlyingReceive(uint8_t *data, uint32_t size);

    /*!
     * @brief Write data to SPI peripheral.
     *
     * @param[in] data Buffer to send.
     * @param[in] data Size of data to send.
     *
     * @retval kErpcStatus_SendFailed SPI failed to send data.
     * @retval kErpcStatus_Success Successfully sent all data.
     */
    virtual erpc_status_t underlyingSend(const uint8_t *data, uint32_t size);
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__SPI_SLAVE_TRANSPORT_H_
