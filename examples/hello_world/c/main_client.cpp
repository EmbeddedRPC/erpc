#include "erpc_c/setup/erpc_client_setup.h"
#include "erpc_c/setup/erpc_transport_setup.h"
#include "erpc_c/setup/erpc_mbf_setup.h"
#include "examples/hello_world/shim/c/hello_world_client.hpp"
#include "examples/config.h"

int main()
{
    erpc_transport_t transport;
    erpc_mbf_t message_buffer_factory;
    erpc_client_t client_manager;

    /* Init eRPC client infrastructure */
    transport = erpc_transport_tcp_init(ERPC_HOSTNAME, ERPC_PORT, false);
    message_buffer_factory = erpc_mbf_dynamic_init();
    client_manager = erpc_client_init(transport, message_buffer_factory);

    /* scope for client service */
    {
        /* init eRPC client TextService service */
        TextService_client client(client_manager);

        /* do eRPC call */
        client.printText("Hello world!");
    }

    /* deinit objects */
    erpc_client_deinit(client_manager);
    erpc_mbf_dynamic_deinit(message_buffer_factory);
    erpc_transport_tcp_deinit(transport);
}
