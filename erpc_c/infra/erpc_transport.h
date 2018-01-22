/*
 * The Clear BSD License
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 * that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
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

#ifndef _EMBEDDED_RPC__TRANSPORT_H_
#define _EMBEDDED_RPC__TRANSPORT_H_

#include "erpc_common.h"
#include "erpc_crc16.h"
#include "erpc_message_buffer.h"
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
    virtual void setCrc16(Crc16 *crcImpl){};
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
