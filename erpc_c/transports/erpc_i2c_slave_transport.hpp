/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__I2C_SLAVE_TRANSPORT_H_
#define _EMBEDDED_RPC__I2C_SLAVE_TRANSPORT_H_

#include "erpc_config_internal.h"
#include <cstdlib>
#if ERPC_THREADS
#include "erpc_threading.h"
#endif
#include "erpc_framed_transport.hpp"

extern "C" {
#include "fsl_gpio.h"
#include "fsl_i2c.h"
}

/*!
 * @addtogroup i2c_slave_transport
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc {
/*!
 * @brief Very basic transport to send/receive messages via I2C.
 *
 * @ingroup i2c_slave_transport
 */
class I2cSlaveTransport : public FramedTransport
{
public:
    /*!
     * @brief Constructor.
     *
     * @param[in] i2cBaseAddr I2C peripheral base address.
     * @param[in] baudRate Baudrate.
     * @param[in] srcClock_Hz Source clock.
     */
    I2cSlaveTransport(I2C_Type *i2cBaseAddr, uint32_t baudRate, uint32_t srcClock_Hz);

    /*!
     * @brief Destructor.
     */
    virtual ~I2cSlaveTransport(void);

    /*!
     * @brief Initialize I2C peripheral configuration structure with values specified in I2cTransport constructor.
     *
     * @retval kErpcStatus_Success Always returns success status.
     */
    virtual erpc_status_t init(void);

    /*!
     * @brief Function called from I2C_SlaveUserCallback when I2C transfer is completed
     *
     * Unblocks the send/receive function.
     */
    void transfer_cb(void);

protected:
    I2C_Type *m_i2cBaseAddr; /*!< Base address of I2C peripheral used in this transport layer */
    uint32_t m_baudRate;     /*!< Baud rate of I2C peripheral used in this transport layer */
    uint32_t m_srcClock_Hz;  /*!< Source clock of I2C peripheral used in this transport layer */
    bool m_isInited;         /*!< the I2C peripheral init status flag */
#if ERPC_THREADS
    Semaphore m_txrxSemaphore; /*!< Semaphore used by RTOS to block task until the sending/receiving is not complete */
#endif

private:
    using FramedTransport::underlyingReceive;
    using FramedTransport::underlyingSend;

    /*!
     * @brief Receive data from I2C peripheral.
     *
     * @param[inout] data Preallocated buffer for receiving data.
     * @param[in] size Size of data to read.
     *
     * @retval kErpcStatus_ReceiveFailed I2C failed to receive data.
     * @retval kErpcStatus_Success Successfully received all data.
     */
    virtual erpc_status_t underlyingReceive(uint8_t *data, uint32_t size) override;

    /*!
     * @brief Write data to I2C peripheral.
     *
     * @param[in] data Buffer to send.
     * @param[in] size Size of data to send.
     *
     * @retval kErpcStatus_SendFailed I2C failed to send data.
     * @retval kErpcStatus_Success Successfully sent all data.
     */
    virtual erpc_status_t underlyingSend(const uint8_t *data, uint32_t size) override;
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__I2C_SLAVE_TRANSPORT_H_
