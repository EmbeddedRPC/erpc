/*
 * test_erpcdemo_server.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <erpc_server_setup.h>
#include "erpcdemo_server.h"
#include "erpc_transport_setup.h"

#define RESPONSE_STR    "I am fine"

/** The servicer end implements the interface method */
binary_t * RD_demoHello(const binary_t * txInput)
{
	printf("Server input: %s\n", txInput->data);

	char* buf = (char*)malloc(strlen(RESPONSE_STR) + 1);
	strncpy(buf, RESPONSE_STR, strlen(RESPONSE_STR));

    binary_t *foo = malloc(sizeof(binary_t));
    foo->data = buf;
    foo->dataLength = strlen(RESPONSE_STR) + 1;

    return foo;
}

int main(int argc, char *argv[])
{
	// Create a client-side transport layer object (TCP), 127.0.0.1:5407
	erpc_transport_t transport = erpc_transport_tcp_init("127.0.0.1", 5407, true);

	/* MessageBufferFactory initialization */
	erpc_mbf_t message_buffer_factory = erpc_mbf_dynamic_init();

	/* eRPC server initialization */
	erpc_server_init(transport, message_buffer_factory);

	/** Add the generated interface service DEMO to the server, see the generated source file erpcdemo_server.h */
	erpc_add_service_to_server(create_DEMO_service());

	printf("Starting server\n");

	/* Start the server */
    for (int i = 0; i < 3; i++)
    {
        erpc_server_run(); /* or erpc_server_poll(); */
    }

	/* Close the socket */
    erpc_transport_tcp_close();

	return 0;
}
