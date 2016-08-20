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

#ifndef _EMBEDDED_RPC__UART_TRANSPORT_H_
#define _EMBEDDED_RPC__UART_TRANSPORT_H_

#include <stdlib.h>
#include "framed_transport.h"
#include "fsl_uart.h"

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
    UartTransport(UART_Type *uartBaseAddr, uint32_t baudRate, uint32_t srcClock_Hz);
    virtual ~UartTransport();

    /*!
     * @brief Initialize UART peripheral configuration structure with values specified in UartTransport constructor.
     *
     * @retval kErpcStatus_Success When data was read successfully.
     * @retval kStatus_UART_BaudrateNotSupport When unacceptable baud rate difference is more than 3%.
     */
    virtual status_t init();

protected:
    UART_Type *m_uartBaseAddr; /*!< Base address of UART peripheral used in this transport layer */
    uint32_t m_baudRate;       /*!< Baud rate of UART peripheral used in this transport layer */
    uint32_t m_srcClock_Hz;    /*!< Source clock of UART peripheral used in this transport layer */

    uart_config_t m_uartConfig; /*!< UART peripheral configuration structure */

private:
    /*!
     * @brief Receive data from UART peripheral.
     *
     * @param[inout] data Preallocated buffer for receiving data.
     * @param[in] data Size of data to read.
     *
     * @retval kStatus_UART_RxHardwareOverrun Receiver overrun happened while receiving data.
     * @retval kStatus_UART_NoiseError Noise error happened while receiving data.
     * @retval kStatus_UART_FramingError Framing error happened while receiving data.
     * @retval kStatus_UART_ParityError Parity error happened while receiving data.
     * @retval kErpcStatus_Success Successfully received all data.
     */
    virtual status_t underlyingReceive(uint8_t *data, uint32_t size);

    /*!
     * @brief Write data to UART peripheral.
     *
     * @param[in] data Buffer to send.
     * @param[in] data Size of data to send.
     *
     * @retval kErpcStatus_Success Always returns success status.
     */
    virtual status_t underlyingSend(const uint8_t *data, uint32_t size);
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__UART_TRANSPORT_H_
