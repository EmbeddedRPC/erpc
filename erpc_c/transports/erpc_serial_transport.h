/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__SERIAL_TRANSPORT_H_
#define _EMBEDDED_RPC__SERIAL_TRANSPORT_H_

#include "erpc_framed_transport.h"

#include <string>

#ifdef _WIN32
typedef long speed_t;
#else
#include <termios.h>
#endif

/*!
 * @addtogroup serial_transport
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc {
/*!
 * @brief Serial transport layer for host PC
 *
 * @ingroup serial_transport
 */
class SerialTransport : public FramedTransport
{
public:
    /*!
     * @brief Constructor.
     *
     * @param[in] portName Port name.
     * @param[in] baudRate Baudrate.
     */
    SerialTransport(const char *portName, speed_t baudRate);

    /*!
     * @brief Destructor.
     */
    virtual ~SerialTransport(void);

    /*!
     * @brief Initialize Serial peripheral.
     *
     * @param[in] vtime Read timeout.
     * @param[in] vmin Read timeout min.
     *
     * @return Status of init function.
     */
    erpc_status_t init(uint8_t vtime, uint8_t vmin);

private:
    /*!
     * @brief Write data to Serial peripheral.
     *
     * @param[in] data Buffer to send.
     * @param[in] size Size of data to send.
     *
     * @retval kErpcStatus_ReceiveFailed Serial failed to receive data.
     * @retval kErpcStatus_Success Successfully received all data.
     */
    virtual erpc_status_t underlyingSend(const uint8_t *data, uint32_t size);

    /*!
     * @brief Receive data from Serial peripheral.
     *
     * @param[inout] data Preallocated buffer for receiving data.
     * @param[in] size Size of data to read.
     *
     * @retval kErpcStatus_ReceiveFailed Serial failed to receive data.
     * @retval kErpcStatus_Success Successfully received all data.
     */
    virtual erpc_status_t underlyingReceive(uint8_t *data, uint32_t size);

private:
    int m_serialHandle;     /*!< Serial handle id. */
    const char *m_portName; /*!< Port name. */
    speed_t m_baudRate;     /*!< Bauderate. */
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__SERIAL_TRANSPORT_H_
