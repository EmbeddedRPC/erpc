/**
 * @file erpc_setup_lwip_tcp.cpp
 * @brief TCP Transport setup code.
 *
 *
 * @author Andrej Hýroš, xhyros00@stud.fit.vut.cz
 * @date 7th of May, 2025
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include "erpc_transport_setup.h"
#include "erpc_lwip_tcp_transport.hpp"
#include "ethernetif.h"


using namespace erpc;


// TODO Missing support for static allocation policy
erpc_transport_t erpc_transport_lwip_tcp_init(const char *host, uint16_t port, bool isServer) {
	erpc_transport_t transport;
	LwipTCPTransport *lwipTCPTransport;

#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
	lwipTCPTransport = new LwipTCPTransport(host, port, isServer);
#else
#error "Only dynamic allocation policy supported for lwipTCPTransport!"
#endif

	transport = reinterpret_cast<erpc_transport_t>(lwipTCPTransport);

	if(lwipTCPTransport != NULL) {
		if(lwipTCPTransport->openTransport() != kErpcStatus_Success) {
			LWIPTCP_DEBUG_PRINT("[erpc_transport_lwip_tcp_init()] Setup failed.");
			transport = NULL;
		}
	}

	return transport;
}


void erpc_transport_lwip_tcp_close(erpc_transport_t transport) {
    erpc_assert(transport != NULL);

    LwipTCPTransport *lwipTcpTransport = reinterpret_cast<LwipTCPTransport *>(transport);

    lwipTcpTransport->closeTransport();
}

void erpc_transport_lwip_tcp_deinit(erpc_transport_t transport) {
#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    erpc_assert(transport != NULL);

    LwipTCPTransport *lwipTcpTransport = reinterpret_cast<LwipTCPTransport *>(transport);

    delete lwipTcpTransport;
#else
#error "Only dynamic allocation policy supported for lwipTCPTransport!"
#endif
}
