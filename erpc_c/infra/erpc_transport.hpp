/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__TRANSPORT_H_
#define _EMBEDDED_RPC__TRANSPORT_H_

#include "erpc_common.h"
#include "erpc_crc16.hpp"
#include "erpc_message_buffer.hpp"

#include <cstring>

/*!
 * @addtogroup infra_transport
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc {

/*!
 * @brief Abstract interface for transport layer.
 *
 * @ingroup infra_transport
 */
class Transport
{
public:
    /*!
     * @brief Constructor.
     */
    Transport(void) {}

    /*!
     * @brief Transport destructor
     */
    virtual ~Transport(void) {}

    /*!
     * @brief Prototype for receiving message.
     *
     * Each transport layer need define this function.
     *
     * @param[out] message Will return pointer to received message buffer.
     *
     * @return based on receive implementation.
     */
    virtual erpc_status_t receive(MessageBuffer *message) = 0;

    /*!
     * @brief Prototype for send message.
     *
     * Each transport layer need define this function.
     *
     * @param[in] message Pass message buffer to send.
     *
     * @return based on send implementation.
     */
    virtual erpc_status_t send(MessageBuffer *message) = 0;

    /*!
     * @brief Poll for an incoming message.
     *
     * This function should return true if are some messages to process by server,
     * the return value should be tested before calling receive function to avoid
     * waiting for a new message (receive can be implemented as blocking function).
     *
     * @retval True when a message is available to process, else false.
     */
    virtual bool hasMessage(void) { return true; }

    /*!
     * @brief This functions sets the CRC-16 implementation.
     *
     * @param[in] crcImpl Object containing crc-16 compute function.
     */
    virtual void setCrc16(Crc16 *crcImpl) { (void)crcImpl; }

    /*!
     * @brief This functions gets the CRC-16 object.
     *
     * @return Crc16* Pointer to CRC-16 object containing crc-16 compute function.
     */
    virtual Crc16 *getCrc16(void) { return NULL; }
};

/*!
 * @brief Abstract interface for transport factory.
 *
 * @ingroup infra_transport
 */
class TransportFactory
{
public:
    /*!
     * @brief Constructor.
     */
    TransportFactory(void) {}
    /*!
     * @brief TransportFactory destructor
     */
    virtual ~TransportFactory(void) {}
    /*!
     * @brief Return created transport object.
     *
     * @return Pointer to created transport object.
     */
    virtual Transport *create(void) = 0;
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__TRANSPORT_H_
