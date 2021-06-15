/*
 * Copyright 2017 NXP
 * All rights reserved.
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
 * o Neither the name of the copyright holder nor the names of its
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
 * 
 * This file was copied and renamed from https://github.com/EmbeddedRPC/erpc-imx-demos/tree/master/middleware/rpmsg-cpp
 * Adding the prefix "erpc_" to the filename
 */

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdint.h>

// RPMSG_MAX_DATAGRAM_SIZE can be used for alloc buffer size for DATAGRAM.
//#define RPMSG_MAX_DATAGRAM_SIZE 1<<20

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

class RPMsgEndpoint
{
public:
    enum _endpoint_types
    {
        kDatagram,
        kStream
    } endpoint_types_t;

    enum _endpoint_addresses
    {
        kLocalDefaultAddress = -1,
        kRemoteDefaultAddress = -1,
        kCurrentDSTDefaultAddress = -2
    } endpoint_addresses_t;

    /*!
     * @brief Constructor to set endpoint type, local and remote address.
     *
     * @param[in] new_addr Local address.
     * @param[in] type endpoint type
     * @param[in] remote_addr Remote address.
     */
    RPMsgEndpoint(int16_t new_addr, int8_t type, int16_t remote_addr);
    virtual ~RPMsgEndpoint();

    /*!
     * @brief This function initializes Linux enviroment for sending and receving messages.
     */
    int8_t init();

     /*!
     * @brief This function sends the messages.
     *
     * @param[in] buffer Buffer to send.
     * @param[in] dst Destination address.
     * @param[in] len Size of message to send.
     */
    int32_t send(uint8_t *buffer, int16_t dst, uint16_t len);

    /*!
     * @brief This function receives the messages.
     *
     * @param[in] buffer Buffer for received message.
     * @param[in] len Max size of message to receive.
     */
    int32_t receive(uint8_t *buffer, uint32_t maxlen);

private:
    int16_t m_id; /*!< Channel id */
    int8_t m_type;/*!< Endpoint socket type. */
    int16_t m_addr;/*!< Local address. */
    int16_t m_remote;/*!< Remote address. */
    int16_t m_currentDST;/*!< Current remote destination address. */
    int16_t m_fd; /*!< File descriptor for sending/receiving messages. */
    const char *m_rpmsg_root; /*!< Linux RPMsg root path. */
};
