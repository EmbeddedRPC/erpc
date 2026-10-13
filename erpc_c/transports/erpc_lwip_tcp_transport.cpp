/**
 * @file erpc_lwip_tcp_transport.cpp
 * @brief TCP Transport implementation on LwIP TCP/IP stack.
 *
 *
 * @author Andrej Hýroš, xhyros00@stud.fit.vut.cz
 * @date 7th of May, 2025
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_lwip_tcp_transport.hpp"

using namespace erpc;


LwipTCPTransport::LwipTCPTransport(const char *host, uint16_t port, bool isServer) :
	m_host(host),
	m_isServer(isServer),
	m_port(port),
	m_runServer(false),
	m_clientSock(-1),
	m_serverThread(serverThreadStub, DEFAULT_THREAD_PRIO, DEFAULT_THREAD_STACKSIZE)
{
}

LwipTCPTransport::~LwipTCPTransport() {
	closeTransport();
}



void LwipTCPTransport::configure(const char* host, uint16_t port, bool isServer) {
	m_host = host;
    m_isServer = isServer;
    m_port = port;
}


erpc_status_t LwipTCPTransport::openTransport() {
	erpc_status_t status = kErpcStatus_Success;
	/*
	 * Open transport as a server or as a client.
	 */
	if(m_isServer) {
		LWIPTCP_DEBUG_PRINT("[openTransport()] Starting server...");
		m_runServer = true;
		m_serverThread.start(this);
	} else {
		status = connectClient();
	}
    return status;
}



erpc_status_t LwipTCPTransport::closeTransport() {
	/*
	 * End server loop and close connection to the client.
	 */
	if (m_isServer) {
		m_runServer = false;
	}
	lwip_close(m_clientSock);


	return kErpcStatus_Success;
}





erpc_status_t LwipTCPTransport::connectClient() {


	if(m_clientSock != -1) {
		LWIPTCP_DEBUG_PRINT("[connectClient()] Client is already connected!");
		return kErpcStatus_Success;
	}


	struct sockaddr_in srv;

	/*
	 * Configure and create socket.
	 */
	memset(&srv, 0, sizeof(srv));
	srv.sin_family = AF_INET;
	srv.sin_addr.s_addr = inet_addr(m_host);
	srv.sin_port = htons(m_port);

	m_clientSock = lwip_socket(AF_INET, SOCK_STREAM, 0);
	if(m_clientSock < 0) {
		LWIPTCP_DEBUG_PRINT("[connectClient()] Error creating socket...");
		return kErpcStatus_Fail;
	}

	/*
	 * TCP_NODELAY disables Nagle's algorithm, which should reduce latency for small packets.
	 * Without this setting, TCP would wait for additional data, so it can send multiple packets
	 * together. This helps with congestion controll, but in eRPC case, we often want to send
	 * small messages.
	 */
	int yes = 1;
	lwip_setsockopt(m_clientSock, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes));


	/*
	 * Connect to server.
	 */
	if (lwip_connect(m_clientSock, (struct sockaddr *)&srv, sizeof(srv)) < 0) {
		LWIPTCP_DEBUG_PRINT("[connectClient()] Connection failed...");
		lwip_close(m_clientSock);
		return kErpcStatus_ConnectionFailure;
	}
	return kErpcStatus_Success;
}


erpc_status_t LwipTCPTransport::disconnect() {
	/*
	 * If socket is valid, close it and mark it as invalid.
	 */
    if (m_clientSock >= 0) {
        lwip_close(m_clientSock);
        m_clientSock = -1;
    }
    return kErpcStatus_Success;
}


erpc_status_t LwipTCPTransport::startServer()
{
	struct sockaddr_in srv;
	struct sockaddr_in cli;
	socklen_t cliInfoLen = sizeof(struct sockaddr_in);
	memset(&srv, 0, sizeof(srv));
	memset(&cli, 0, sizeof(cli));
	int incomingSocket;

	/*
	 * Define address.
	 */
	srv.sin_family = AF_INET;
	srv.sin_port = htons(m_port);
	srv.sin_addr.s_addr = INADDR_ANY;

	/*
	 * Create listening socket.
	 */
	if((m_serverSock = lwip_socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		LWIPTCP_DEBUG_PRINT("[startServer()] Error creating socket...");
		return kErpcStatus_Fail;
	}

	/*
	 * SO_REUSEADDR makes the port available right after closing.
	 */
	int yes = 1;
	lwip_setsockopt(m_serverSock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));


	/*
	 * Bind socket to address.
	 */
	if(lwip_bind(m_serverSock, (struct sockaddr *)&srv, sizeof(srv)) == -1) {
		LWIPTCP_DEBUG_PRINT("[startServer()] Failed on bind()...");
		lwip_close(m_serverSock);
		return kErpcStatus_Fail;
	}

	/*
	 * Starting listening.
	 */
	if(lwip_listen(m_serverSock, 1) == -1) {
		LWIPTCP_DEBUG_PRINT("[startServer()] Failed on listen()...");
		lwip_close(m_serverSock);
		return kErpcStatus_Fail;
	}


	while(m_runServer) {
		incomingSocket = lwip_accept(m_serverSock, (struct sockaddr *)&cli, &cliInfoLen);

		/*
		 * In case of accept failure, wait some time and try again.
		 */
		if(incomingSocket < 0) {
			LWIPTCP_DEBUG_PRINT("[startServer()] Failed on accept()...");
			Thread::sleep(1000); // 1ms
			continue;
		} else {
			m_clientSock = incomingSocket;
		}

	}

	lwip_close(m_serverSock);
	m_serverSock = -1;

	return kErpcStatus_Success;
}


erpc_status_t LwipTCPTransport::underlyingReceive(uint8_t *data, uint32_t size) {
	ssize_t read;

	/*
	 * Repeat reading from socket until we read all that we want.
	 */
	while(size > 0) {
		read = lwip_read(m_clientSock, data, size);
		if(read > 0) {
			size -= read;
			data += read;
		}
		/*
		 * Connection was closed by peer.
		 */
		if(read == 0) {
			lwip_close(m_clientSock);
			return kErpcStatus_ConnectionClosed;
		}

		/*
		 * Read error.
		 */
		if(read < 0) {
			return kErpcStatus_ReceiveFailed;
		}
	}
	return kErpcStatus_Success;
}

erpc_status_t LwipTCPTransport::underlyingSend(const uint8_t *data, uint32_t size) {
	ssize_t toSend = size;
	ssize_t sent;


	/*
	 * Socket is invalid.
	 */
	if(m_clientSock < 0) {
		return kErpcStatus_ConnectionFailure;
	}

	/*
	 * Write into socket until we sent all that we wanted.
	 */
	while(toSend > 0) {
		sent = lwip_write(m_clientSock, data, toSend);

		if(sent > 0) {
			toSend -= sent;
			data += sent;
		} else {
			return kErpcStatus_SendFailed;
		}
	}
	return kErpcStatus_Success;
}

bool LwipTCPTransport::hasMessage(void) {
	/*
	 * Return true if server is connected to client.
	 */
	return (m_clientSock >= 0);
}

void LwipTCPTransport::serverThreadStub(void *arg) {
	/*
	 * Start server as a separate task/thread.
	 */
    LwipTCPTransport *instance = reinterpret_cast<LwipTCPTransport *>(arg);
    if (instance != NULL)
    {
    	instance->startServer();
    } else {
    	LWIPTCP_DEBUG_PRINT("[serverThreadStub()] Failed on thread creation...");
    }
}















