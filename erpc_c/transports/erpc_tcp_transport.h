/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _EMBEDDED_RPC__TCP_TRANSPORT_H_
#define _EMBEDDED_RPC__TCP_TRANSPORT_H_

#include "erpc_framed_transport.h"
#include "erpc_threading.h"

/*!
 * @addtogroup tcp_transport
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc {
/*!
 * @brief Client side of TCP/IP transport.
 *
 * @ingroup tcp_transport
 */
class TCPTransport : public FramedTransport
{
public:
    /*!
     * @brief Constructor.
     *
     * This function initializes object attributes.
     *
     * @param[in] isServer True when this transport is used for server side application.
     */
    TCPTransport(bool isServer);

    /*!
     * @brief Constructor.
     *
     * This function initializes object attributes.
     *
     * @param[in] host Specify the host name or IP address of the computer.
     * @param[in] port Specify the listening port number.
     * @param[in] isServer True when this transport is used for server side application.
     */
    TCPTransport(const char *host, uint16_t port, bool isServer);

    /*!
     * @brief TCPTransport destructor
     */
    virtual ~TCPTransport(void);

    /*!
     * @brief This function set host and port of this transport layer.
     *
     * @param[in] host Specify the host name or IP address of the computer.
     * @param[in] port Specify the listening port number.
     */
    void configure(const char *host, uint16_t port);

    /*!
     * @brief This function will create host on server side, or connect client to the server.
     *
     * @retval #kErpcStatus_Success When creating host was successful or client connected successfully.
     * @retval #kErpcStatus_UnknownName Host name resolution failed.
     * @retval #kErpcStatus_ConnectionFailure Connecting to the specified host failed.
     */
    virtual erpc_status_t open(void);

    /*!
     * @brief This function disconnects client or stop server host.
     *
	 * @param[in] stopServer Specify is server shall be closed as well (stop listen())
     * @retval #kErpcStatus_Success Always return this.
     */
    virtual erpc_status_t close(bool stopServer = true);

protected:
    bool m_isServer;       /*!< If true then server is using transport, else client. */
    const char *m_host;    /*!< Specify the host name or IP address of the computer. */
    uint16_t m_port;       /*!< Specify the listening port number. */
    int m_socket;          /*!< Socket number. */
    Thread m_serverThread; /*!< Pointer to server thread. */
    bool m_runServer;      /*!< Thread is executed while this is true. */

    /*!
     * @brief This function connect client to the server.
     *
     * @retval kErpcStatus_Success When client connected successfully.
     * @retval kErpcStatus_Fail When client doesn't connected successfully.
     */
    virtual erpc_status_t connectClient(void);

    /*!
     * @brief This function read data.
     *
     * @param[inout] data Preallocated buffer for receiving data.
     * @param[in] size Size of data to read.
     *
     * @retval #kErpcStatus_Success When data was read successfully.
     * @retval #kErpcStatus_ReceiveFailed When reading data ends with error.
     * @retval #kErpcStatus_ConnectionClosed Peer closed the connection.
     */
    virtual erpc_status_t underlyingReceive(uint8_t *data, uint32_t size);

    /*!
     * @brief This function writes data.
     *
     * @param[in] data Buffer to send.
     * @param[in] size Size of data to send.
     *
     * @retval #kErpcStatus_Success When data was written successfully.
     * @retval #kErpcStatus_SendFailed When writing data ends with error.
     * @retval #kErpcStatus_ConnectionClosed Peer closed the connection.
     */
    virtual erpc_status_t underlyingSend(const uint8_t *data, uint32_t size);

    /*!
     * @brief Server thread function.
     */
    void serverThread(void);

    /*!
     * @brief Thread entry point.
     *
     * Control is passed to the serverThread() method of the TCPTransport instance pointed to
     * by the @c arg parameter.
     *
     * @param arg Thread argument. The pointer to the TCPTransport instance is passed through
     *  this argument.
     */
    static void serverThreadStub(void *arg);
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__TCP_TRANSPORT_H_
