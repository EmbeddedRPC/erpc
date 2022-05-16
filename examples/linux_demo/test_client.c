/*
 * test_erpcdemo_client.c
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <erpc_client_setup.h>
#include <erpc_port.h>
#include "erpcdemo.h"
#include "erpc_mbf_setup.h"
#include "erpc_transport_setup.h"

// Release the space occupied by binary
static void free_binary_t_struct(binary_t * data)
{
    if (data->data)
    {
        erpc_free(data->data);
    }
}

#define QUERY "How are you"

int main(int argc, char *argv[])
{
	/* Create a client-side transport layer object (TCP), 127.0.0.1:5407 */
	erpc_transport_t  transport = erpc_transport_tcp_init("127.0.0.1", 5407, false);
    if (transport == NULL)
    {
        printf("Client: Cannot init transport stream. Is server running?\n");
        exit(0);
    }

	erpc_mbf_t message_buffer_factory = erpc_mbf_dynamic_init();

	/* Initialize the client */
	erpc_client_init(transport, message_buffer_factory);

	char *msg = QUERY;
	binary_t b;
    b.data = msg;
    b.dataLength = strlen(msg);

    /* RPC call. b is on the stack so no need to free it. */
    binary_t *resp = RD_demoHello(&b);

    /* Output return value */
    printf("RD_demoHello response: %s\n", resp->data);

    /* For the data of the return pointer type, the memory allocated in RD_demoHello needs to be released after use */
    free_binary_t_struct(resp);

	/* Close the socket */
	erpc_transport_tcp_close();
}


