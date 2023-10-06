/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
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
