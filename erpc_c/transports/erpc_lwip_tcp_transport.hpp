/**
 * @file erpc_lwip_tcp_transport.hpp
 * @brief TCP Transport implementation on LwIP TCP/IP stack.
 *
 *
 * @author Andrej Hýroš, xhyros00@stud.fit.vut.cz
 * @date 7th of May, 2025
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#ifndef EVKMIMXRT1060_ERPC_LWIP_EXAMPLE_LWIPTCPTRANSPORT_HPP
#define EVKMIMXRT1060_ERPC_LWIP_EXAMPLE_LWIPTCPTRANSPORT_HPP


#include "erpc_framed_transport.hpp"
#include "erpc_threading.h"

#include <cstdio>
#include <string>

// Set this macro to 0 if you do not want debug prints.
#define LWIPTCP_TRANSPORT_DEBUG_LOG 1

#if LWIPTCP_TRANSPORT_DEBUG_LOG
#define LWIPTCP_DEBUG_PRINT(_fmt_, ...) PRINTF(_fmt_ "\n\r", ##__VA_ARGS__)
#endif


extern "C" {
#include "lwip/altcp.h"
#include "lwip/altcp_tcp.h"
#include "lwip/sockets.h"
}


namespace erpc
{


/**
 * @class LwipTCPTransport
 * @brief Transport layer implementation using LwIP over TCP for eRPC.
 *
 * This class provides a framed transport interface over a TCP connection
 * using the Lightweight IP (LwIP) stack. It can operate in both client and
 * server modes depending on configuration.
 */
class LwipTCPTransport : public FramedTransport
{
public:
    /**
     * @brief Constructor for LwipTCPTransport.
     *
     * @param host IP address or hostname to connect to or bind to.
     * @param port Port number for connection.
     * @param isServer Set to true if the instance should act as a server.
     */
    LwipTCPTransport(const char* host, uint16_t port, bool isServer);


    virtual ~LwipTCPTransport();


    /**
     * @brief Configures the transport with new host, port, and mode.
     *
     * @param host IP address or hostname.
     * @param port Port number to use.
     * @param isServer If true, transport acts as a server.
     */
    virtual void configure(const char* host, uint16_t port, bool isServer);


    /**
     * @brief Opens the transport (e.g., starts server thread or connects to the server).
     *
     * @retval kErpcStatus_Success If transport is successfully opened.
     * @retval kErpcStatus_Fail If opening transport fails.
     */
    virtual erpc_status_t openTransport();


    /**
     * @brief Closes the transport (e.g., releases sockets and resources).
     *
     * @retval kErpcStatus_Success If transport is successfully closed.
     * @retval kErpcStatus_Fail If closing transport fails.
     */
    virtual erpc_status_t closeTransport();


    /*!
     * @brief Connects client to server.
     *
     * @retval kErpcStatus_Success Successful connection to the server
     * @retval kErpcStatus_Fail Failed to create socket
     * @retval kErpcStatus_ConnectionFailure Connection attempt failed
     */
    virtual erpc_status_t connectClient();

    // TODO unused.
    virtual erpc_status_t disconnect();


    /*!
     * @brief Starts server.
     *
     * @retval kErpcStatus_Success If server has shutdown correctly
     * @retval kErpcStatus_Fail If initialisation of server failed
     * @retval kErpcStatus_ConnectionFailure accept() call failed
     */
    virtual erpc_status_t startServer();


    /*!
     * @brief Receive data from lwIP TCP/IP stack using socket API.
     *
     *
     * @param[inout] data Preallocated buffer for receiving data.
     * @param[in] size Size of data to read.
     *
     * @retval kErpcStatus_Connection Closed Connection was closed by peer.
     * @retval kErpcStatus_Success Successfully received all data.
     * @retval kErpcStatus_ReceiveFailed read() call failed.
     */
    virtual erpc_status_t underlyingReceive(uint8_t *data, uint32_t size) override;


    /*!
     * @brief Write data to lwIP socket.
     *
     * @param[in] data Buffer to send.
     * @param[in] size Size of data to send.
     *
     * @retval kErpcStatus_Success Data was sent successfully.
     * @retval kErpcStatus_SendFailed Failed to send data.
     */
    virtual erpc_status_t underlyingSend(const uint8_t *data, uint32_t size) override;


    /*!
     * @brief Returns true if connection is established.
     * This prevents blocking of the task that is polling the eRPC server
     * when there is no connection established. However, if connection is
     * established, then said task will be blocked on underlyingReceive's
     * read() call until requested amount of data came through the socket.
     * Name of this method is therefore not accurate.
     */
    virtual bool hasMessage(void);


    /*!
     * @brief Stub for server thread (only static methods can be made tasks)
     *
     * @param[in] arg Instance of this class is passed through this argument to call server method.
     */
    static void serverThreadStub(void *arg);


protected:
    const char *m_host; 	/*!< String specifying host. Can only be IP address at the moment.
							Irrelevant if m_isServer is true */
    bool m_isServer;		/*!< Instance's role. */
    uint16_t m_port; 		/*!< Specify the listening port number. */
    bool m_runServer;		/*!< Indicates server state. */

    int m_clientSock;		/*!< Socket connected to the peer */
	int m_serverSock;		/*!< Server's listening socket. Unused in client mode */

    Thread m_serverThread;	/*!< Server thread. */
};

} // erpc

#endif //EVKMIMXRT1060_ERPC_LWIP_EXAMPLE_LWIPTCPTRANSPORT_HPP
