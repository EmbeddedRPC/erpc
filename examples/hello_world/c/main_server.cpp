#include "erpc_c/setup/erpc_server_setup.h"
#include "erpc_c/setup/erpc_transport_setup.h"
#include "erpc_c/setup/erpc_mbf_setup.h"
#include "examples/hello_world/shim/c/hello_world_server.hpp"
#include "examples/config.h"
#include <cstdio>

class TextService : public TextService_interface
{
    /* eRPC call definition */
    void printText(const char *text) override { printf("%s", text); }
}

int main()
{
    erpc_transport_t transport;
    erpc_mbf_t message_buffer_factory;
    erpc_server_t server;
    TextService textServiceImpl;
    TextService_service textService(&textServiceImpl);

    /* Init eRPC server infrastructure */
    transport = erpc_transport_tcp_init(ERPC_HOSTNAME, ERPC_PORT, true);
    message_buffer_factory = erpc_mbf_dynamic_init();
    server = erpc_server_init(transport, message_buffer_factory);

    /* add custom service implementation to the server */
    erpc_add_service_to_server(server, &textService);

    /* poll for requests */
    erpc_status_t err = server.poll();

    /* deinit objects */
    erpc_server_deinit(server);
    erpc_mbf_dynamic_deinit(message_buffer_factory);
    erpc_transport_tcp_deinit(transport);
}
