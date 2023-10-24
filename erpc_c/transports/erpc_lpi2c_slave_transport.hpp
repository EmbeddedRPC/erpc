/*
 * Copyright 2022-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__LPI2C_SLAVE_TRANSPORT_H_
#define _EMBEDDED_RPC__LPI2C_SLAVE_TRANSPORT_H_

#include "erpc_config_internal.h"
#if ERPC_THREADS
#include "erpc_threading.h"
#endif
#include "erpc_framed_transport.hpp"

extern "C" {
#include "fsl_gpio.h"
#include "fsl_lpi2c.h"
}

/*!
 * @addtogroup lpi2c_slave_transport
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc {
/*!
 * @brief Very basic transport to send/receive messages via LPI2C.
 *
 * @ingroup lpi2c_slave_transport
 */
class LPI2cSlaveTransport : public FramedTransport
{
public:
    /*!
     * @brief Constructor.
     *
     * @param[in] lpi2cBaseAddr LPI2C peripheral base address.
     * @param[in] baudRate Baudrate.
     * @param[in] srcClock_Hz Source clock.
     */
    LPI2cSlaveTransport(LPI2C_Type *lpi2cBaseAddr, uint32_t baudRate, uint32_t srcClock_Hz);

    /*!
     * @brief Destructor.
     */
    virtual ~LPI2cSlaveTransport(void);

    /*!
     * @brief Initialize LPI2C peripheral configuration structure with values specified in LPI2cTransport constructor.
     *
     * @retval kErpcStatus_Success Always returns success status.
     */
    virtual erpc_status_t init(void);

    /*!
     * @brief Function called from LPI2C_SlaveUserCallback when LPI2C transfer is completed
     *
     * Unblocks the send/receive function.
     */
    void transfer_cb(void);

protected:
    LPI2C_Type *m_lpi2cBaseAddr; /*!< Base address of LPI2C peripheral used in this transport layer */
    uint32_t m_baudRate;         /*!< Baud rate of LPI2C peripheral used in this transport layer */
    uint32_t m_srcClock_Hz;      /*!< Source clock of LPI2C peripheral used in this transport layer */
    bool m_isInited;             /*!< the LPI2C peripheral init status flag */
#if ERPC_THREADS
    Semaphore m_txrxSemaphore; /*!< Semaphore used by RTOS to block task until the sending/receiving is not complete */
#endif

private:
    using FramedTransport::underlyingReceive;
    using FramedTransport::underlyingSend;

    /*!
     * @brief Receive data from LPI2C peripheral.
     *
     * @param[inout] data Preallocated buffer for receiving data.
     * @param[in] size Size of data to read.
     *
     * @retval kErpcStatus_ReceiveFailed LPI2C failed to receive data.
     * @retval kErpcStatus_Success Successfully received all data.
     */
    virtual erpc_status_t underlyingReceive(uint8_t *data, uint32_t size) override;

    /*!
     * @brief Write data to LPI2C peripheral.
     *
     * @param[in] data Buffer to send.
     * @param[in] size Size of data to send.
     *
     * @retval kErpcStatus_SendFailed LPI2C failed to send data.
     * @retval kErpcStatus_Success Successfully sent all data.
     */
    virtual erpc_status_t underlyingSend(const uint8_t *data, uint32_t size) override;
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__LPI2C_SLAVE_TRANSPORT_H_
