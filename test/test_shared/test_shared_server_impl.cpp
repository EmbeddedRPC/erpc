/*
 * Copyright 2017 - 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_server_setup.h"

#include "c_test_server.h"
#include "test_server.hpp"
#include "c_test_unit_test_common_server.h"
#include "unit_test.h"
#include "unit_test_wrapped.h"

#include <stdlib.h>

using namespace erpc;

SharedService_service *svc;

////////////////////////////////////////////////////////////////////////////////
// Implementation of function code
////////////////////////////////////////////////////////////////////////////////

/* typedef unit tests */
BaseSharedStruct *sendReceiveBaseSharedStruct(const BaseSharedStruct *s)
{
    return (BaseSharedStruct *)s;
}

void inoutBaseSharedStruct(BaseSharedStruct **s) {}
/* end typedef unit tests */

class SharedService_server: public SharedService_interface
{
    public:

        BaseSharedStruct * sendReceiveBaseSharedStruct(const BaseSharedStruct * a)
        {
            BaseSharedStruct * result = NULL;
            result = ::sendReceiveBaseSharedStruct(a);

            return result;
        }

        void inoutBaseSharedStruct(BaseSharedStruct ** a)
        {
            ::inoutBaseSharedStruct(a);
        }
};

////////////////////////////////////////////////////////////////////////////////
// Add service to server code
////////////////////////////////////////////////////////////////////////////////

void add_services(erpc::SimpleServer *server)
{
    // define services to add on heap
    // allocate on heap so service doesn't go out of scope at end of method
    svc = new SharedService_service(new SharedService_server());

    // add services
    server->addService(svc);
}

////////////////////////////////////////////////////////////////////////////////
// Remove service from server code
////////////////////////////////////////////////////////////////////////////////

void remove_services(erpc::SimpleServer *server)
{
    /* Remove services
     * Example: server->removeService (svc);
     */
    server->removeService(svc);
    /* Delete unused service
     */
    delete svc;
}

#ifdef __cplusplus
extern "C" {
#endif
erpc_service_t service_test = NULL;
void add_services_to_server(erpc_server_t server)
{
    service_test = create_SharedService_service();
    erpc_add_service_to_server(server, service_test);
}

void remove_services_from_server(erpc_server_t server)
{
    erpc_remove_service_from_server(server, service_test);
    destroy_SharedService_service(service_test);
}

#ifdef __cplusplus
}
#endif
