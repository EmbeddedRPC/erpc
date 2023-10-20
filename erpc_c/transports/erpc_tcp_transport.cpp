/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * Copyright 2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "erpc_tcp_transport.hpp"

#include <cstdio>
#include <string>

extern "C" {
// Set this to 1 to enable debug logging.
// TODO fix issue with the transport not working on Linux if debug logging is disabled.
//#define TCP_TRANSPORT_DEBUG_LOG (1)

#if TCP_TRANSPORT_DEBUG_LOG
#if ERPC_HAS_POSIX
#if defined(__MINGW32__)
#error Missing implementation for mingw.
#endif
#include <err.h>
#endif
#endif
#include <errno.h>
#if defined(__MINGW32__)
#include <ws2tcpip.h>
#include <ws2def.h>
#else
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#endif
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
}

using namespace erpc;

#if TCP_TRANSPORT_DEBUG_LOG
#define TCP_DEBUG_PRINT(_fmt_, ...) printf(_fmt_, ##__VA_ARGS__)
#define TCP_DEBUG_ERR(_msg_) err(errno, _msg_)
#else
#define TCP_DEBUG_PRINT(_fmt_, ...)
#define TCP_DEBUG_ERR(_msg_)
#endif

#if defined(__MINGW32__)
#ifndef AI_NUMERICSERV
#define AI_NUMERICSERV 0x00000008 // Servicename must be a numeric port number
#endif
#endif

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

TCPTransport::TCPTransport(bool isServer) :
m_isServer(isServer), m_host(NULL), m_port(0)
#if defined(__MINGW32__)
,
m_socket(INVALID_SOCKET)
#else
,
m_socket(-1)
#endif
,
m_serverThread(serverThreadStub), m_runServer(true)
{
#if defined(__MINGW32__)
    WSADATA ws;
    WSAStartup(MAKEWORD(2, 2), &ws);
#endif
}

TCPTransport::TCPTransport(const char *host, uint16_t port, bool isServer) :
m_isServer(isServer), m_host(host), m_port(port)
#if defined(__MINGW32__)
,
m_socket(INVALID_SOCKET)
#else
,
m_socket(-1)
#endif
,
m_serverThread(serverThreadStub), m_runServer(true)
{
#if defined(__MINGW32__)
    WSADATA ws;
    WSAStartup(MAKEWORD(2, 2), &ws);
#endif
}

TCPTransport::~TCPTransport(void) {}

void TCPTransport::configure(const char *host, uint16_t port)
{
    m_host = host;
    m_port = port;
}

erpc_status_t TCPTransport::open(void)
{
    erpc_status_t status;

    if (m_isServer)
    {
        m_runServer = true;
        m_serverThread.start(this);
        status = kErpcStatus_Success;
    }
    else
    {
        status = connectClient();
    }

    return status;
}

erpc_status_t TCPTransport::connectClient(void)
{
    erpc_status_t status = kErpcStatus_Success;
    struct addrinfo hints = {};
    char portString[8];
    struct addrinfo *res0;
    int result, set;
#if defined(__MINGW32__)
    SOCKET sock = INVALID_SOCKET;
#else
    int sock = -1;
#endif
    struct addrinfo *res;

#if defined(__MINGW32__)
    if (m_socket != INVALID_SOCKET)
#else
    if (m_socket != -1)
#endif
    {
        TCP_DEBUG_PRINT("%s", "socket already connected\n");
    }
    else
    {
        // Fill in hints structure for getaddrinfo.
        hints.ai_flags = AI_NUMERICSERV;
        hints.ai_family = PF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        // Convert port number to a string.
        result = snprintf(portString, sizeof(portString), "%d", m_port);
        if (result < 0)
        {
            TCP_DEBUG_ERR("snprintf failed");
            status = kErpcStatus_Fail;
        }

        if (status == kErpcStatus_Success)
        {
            // Perform the name lookup.
            result = getaddrinfo(m_host, portString, &hints, &res0);
            if (result != 0)
            {
                // TODO check EAI_NONAME
                TCP_DEBUG_ERR("gettaddrinfo failed");
                status = kErpcStatus_UnknownName;
            }
        }

        if (status == kErpcStatus_Success)
        {
            // Iterate over result addresses and try to connect. Exit the loop on the first successful
            // connection.
            for (res = res0; res; res = res->ai_next)
            {
                // Create the socket.
                sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
#if defined(__MINGW32__)
                if (sock == INVALID_SOCKET)
#else
                if (sock < 0)
#endif
                {
                    continue;
                }

                // Attempt to connect.
                if (connect(sock, res->ai_addr, res->ai_addrlen) < 0)
                {
#if defined(__MINGW32__)
                    closesocket(sock);
                    sock = INVALID_SOCKET;
#else
                    ::close(sock);
                    sock = -1;
#endif
                    continue;
                }

                // Exit the loop for the first successful connection.
                break;
            }

            // Free the result list.
            freeaddrinfo(res0);

            // Check if we were able to open a connection.
#if defined(__MINGW32__)
            if (sock == INVALID_SOCKET)
#else
            if (sock < 0)
#endif
            {
                // TODO check EADDRNOTAVAIL:
                TCP_DEBUG_ERR("connecting failed");
                status = kErpcStatus_ConnectionFailure;
            }
        }

        if (status == kErpcStatus_Success)
        {
            set = 1;
#if defined(__MINGW32__)
            if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char *)&set, sizeof(int)) < 0)
#else
            if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void *)&set, sizeof(int)) < 0)
#endif
            {
#if defined(__MINGW32__)
                closesocket(sock);
#else
                ::close(sock);
#endif
                TCP_DEBUG_ERR("setsockopt failed");
                status = kErpcStatus_Fail;
            }
        }

        if (status == kErpcStatus_Success)
        {
// On some systems (BSD) we can disable SIGPIPE on the socket. For others (Linux), we have to
// ignore SIGPIPE.
#if defined(SO_NOSIGPIPE)

            // Disable SIGPIPE for this socket. This will cause write() to return an EPIPE statusor if the
            // other side has disappeared instead of our process receiving a SIGPIPE.
            set = 1;
#if defined(__MINGW32__)
            if (setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, (const char *)&set, sizeof(int)) < 0)
#else
            if (setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int)) < 0)
#endif
            {
#if defined(__MINGW32__)
                closesocket(sock);
#else
                ::close(sock);
#endif
                TCP_DEBUG_ERR("setsockopt failed");
                status = kErpcStatus_Fail;
            }
        }

        if (status == kErpcStatus_Success)
        {
#else
            // globally disable the SIGPIPE signal
#if !defined(__MINGW32__)
            signal(SIGPIPE, SIG_IGN);
#endif
#endif // defined(SO_NOSIGPIPE)
            m_socket = sock;
        }
    }

    return status;
}

erpc_status_t TCPTransport::close(bool stopServer)
{
    if (m_isServer && stopServer)
    {
        m_runServer = false;
    }

#if defined(__MINGW32__)
    if (m_socket != INVALID_SOCKET)
    {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
#else
    if (m_socket != -1)
    {
        ::close(m_socket);
        m_socket = -1;
    }
#endif

    return kErpcStatus_Success;
}

erpc_status_t TCPTransport::underlyingReceive(uint8_t *data, uint32_t size)
{
    ssize_t length;
    erpc_status_t status = kErpcStatus_Success;

    // Block until we have a valid connection.
#if defined(__MINGW32__)
    while (m_socket == INVALID_SOCKET)
#else
    while (m_socket <= 0)
#endif
    {
        // Sleep 10 ms.
        Thread::sleep(10000);
    }

    // Loop until all requested data is received.
    while (size > 0U)
    {
#if defined(__MINGW32__)
        length = recv(m_socket, (char *)data, size, 0);
#else
        length = read(m_socket, data, size);
#endif

        // Length will be zero if the connection is closed.
        if (length > 0)
        {
            size -= length;
            data += length;
        }
        else
        {
            if (length == 0)
            {
                // close socket, not server
                close(false);
                status = kErpcStatus_ConnectionClosed;
            }
            else
            {
                status = kErpcStatus_ReceiveFailed;
            }
            break;
        }
    }

    return status;
}

erpc_status_t TCPTransport::underlyingSend(const uint8_t *data, uint32_t size)
{
    erpc_status_t status = kErpcStatus_Success;
    ssize_t result;

#if defined(__MINGW32__)
    if (m_socket == INVALID_SOCKET)
#else
    if (m_socket <= 0)
#endif
    {
        // we should not pretend to have a succesful Send or we create a deadlock
        status = kErpcStatus_ConnectionFailure;
    }
    else
    {
        // Loop until all data is sent.
        while (size > 0U)
        {
#if defined(__MINGW32__)
            result = ::send(m_socket, (const char *)data, size, 0);
#else
            result = write(m_socket, data, size);
#endif
            if (result >= 0)
            {
                size -= result;
                data += result;
            }
            else
            {
                if (result == EPIPE)
                {
                    // close socket, not server
                    close(false);
                    status = kErpcStatus_ConnectionClosed;
                }
                else
                {
                    status = kErpcStatus_SendFailed;
                }
                break;
            }
        }
    }

    return status;
}

void TCPTransport::serverThread(void)
{
    int yes = 1;
#if defined(__MINGW32__)
    SOCKET serverSocket;
#else
    int serverSocket;
#endif
    int result;
    struct sockaddr incomingAddress;
    socklen_t incomingAddressLength;
    int incomingSocket;
    bool status = false;
    struct sockaddr_in serverAddress;

    TCP_DEBUG_PRINT("%s", "in server thread\n");

    // Create socket.
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
#if defined(__MINGW32__)
    if (serverSocket == INVALID_SOCKET)
#else
    if (serverSocket < 0)
#endif
    {
        TCP_DEBUG_ERR("failed to create server socket");
    }
    else
    {
        // Fill in address struct.
        (void)memset(&serverAddress, 0, sizeof(serverAddress));
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = INADDR_ANY; // htonl(local ? INADDR_LOOPBACK : INADDR_ANY);
        serverAddress.sin_port = htons(m_port);

        // Turn on reuse address option.
#if defined(__MINGW32__)
        result = setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes));
#else
        result = setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const void *)&yes, sizeof(yes));
#endif
        if (result < 0)
        {
            TCP_DEBUG_ERR("setsockopt failed");
            status = true;
        }

        if (!status)
        {
            // Bind socket to address.
            result = bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
            if (result < 0)
            {
                TCP_DEBUG_ERR("bind failed");
                status = true;
            }
        }

        if (!status)
        {
            // Listen for connections.
            result = listen(serverSocket, 1);
            if (result < 0)
            {
                TCP_DEBUG_ERR("listen failed");
                status = true;
            }
        }

        if (!status)
        {
            TCP_DEBUG_PRINT("%s", "Listening for connections\n");

            while (m_runServer)
            {
                incomingAddressLength = sizeof(struct sockaddr);
                // we should use select() otherwise we can't end the server properly
                incomingSocket = accept(serverSocket, &incomingAddress, &incomingAddressLength);
#if defined(__MINGW32__)
                if (incomingSocket != INVALID_SOCKET)
#else
                if (incomingSocket > 0)
#endif
                {
                    // Successfully accepted a connection.
                    m_socket = incomingSocket;
                    // should be inherited from accept() socket but it's not always ...
                    yes = 1;
#if defined(__MINGW32__)
                    setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, (const char *)&yes, sizeof(yes));
#else
                    setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, (void *)&yes, sizeof(yes));
#endif
                }
                else
                {
                    TCP_DEBUG_ERR("accept failed");
                }
            }
        }
#if defined(__MINGW32__)
        closesocket(serverSocket);
#else
        ::close(serverSocket);
#endif
    }
}

void TCPTransport::serverThreadStub(void *arg)
{
    TCPTransport *This = reinterpret_cast<TCPTransport *>(arg);

    TCP_DEBUG_PRINT("in serverThreadStub (arg=%p)\n", arg);
    if (This != NULL)
    {
        This->serverThread();
    }
}
