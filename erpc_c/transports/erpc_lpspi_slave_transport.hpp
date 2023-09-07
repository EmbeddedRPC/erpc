/*
 * Copyright 2022 NXP
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__LPSPI_SLAVE_TRANSPORT_H_
#define _EMBEDDED_RPC__LPSPI_SLAVE_TRANSPORT_H_

#include "erpc_config_internal.h"

#include <cstdlib>
#if ERPC_THREADS
#include "erpc_threading.h"
#endif
#include "erpc_framed_transport.hpp"

extern "C" {
#include "fsl_gpio.h"
#include "fsl_lpspi.h"
}

/*!
 * @addtogroup lpspi_slave_transport
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc {
/*!
 * @brief Very basic transport to send/receive messages via LPSPI.
 *
 * @ingroup lpspi_slave_transport
 */
class LPSpiSlaveTransport : public FramedTransport
{
public:
    /*!
     * @brief Constructor.
     *
     * @param[in] lpspiBaseAddr LPSPI peripheral base address.
     * @param[in] baudRate Baudrate.
     * @param[in] srcClock_Hz Source clock.
     */
    LPSpiSlaveTransport(LPSPI_Type *lpspiBaseAddr, uint32_t baudRate, uint32_t srcClock_Hz);

    /*!
     * @brief Destructor.
     */
    virtual ~LPSpiSlaveTransport(void);

    /*!
     * @brief Initialize LPSPI peripheral configuration structure with values specified in LPSpiTransport constructor.
     *
     * @retval kErpcStatus_Success Always returns success status.
     */
    virtual erpc_status_t init(void);

    /*!
     * @brief Function called from LPSPI_SlaveUserCallback when LPSPI transfer is completed
     *
     * Unblocks the send/receive function.
     */
    void transfer_cb(void);

protected:
    LPSPI_Type *m_lpspiBaseAddr; /*!< Base address of LPSPI peripheral used in this transport layer */
    uint32_t m_baudRate;         /*!< Baud rate of LPSPI peripheral used in this transport layer */
    uint32_t m_srcClock_Hz;      /*!< Source clock of LPSPI peripheral used in this transport layer */
    bool m_isInited;             /*!< the LPSPI peripheral init status flag */
#if ERPC_THREADS
    Semaphore m_txrxSemaphore; /*!< Semaphore used by RTOS to block task until the sending/receiving is not complete */
#endif

private:
    /*!
     * @brief Receive data from LPSPI peripheral.
     *
     * @param[inout] data Preallocated buffer for receiving data.
     * @param[in] size Size of data to read.
     *
     * @retval kErpcStatus_ReceiveFailed LPSPI failed to receive data.
     * @retval kErpcStatus_Success Successfully received all data.
     */
    virtual erpc_status_t underlyingReceive(uint8_t *data, uint32_t size);

    /*!
     * @brief Write data to LPSPI peripheral.
     *
     * @param[in] data Buffer to send.
     * @param[in] size Size of data to send.
     *
     * @retval kErpcStatus_SendFailed LPSPI failed to send data.
     * @retval kErpcStatus_Success Successfully sent all data.
     */
    virtual erpc_status_t underlyingSend(const uint8_t *data, uint32_t size);
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__LPSPI_SLAVE_TRANSPORT_H_
