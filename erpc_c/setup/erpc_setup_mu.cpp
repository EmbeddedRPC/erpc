/*
 * Copyright 2017-2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_manually_constructed.hpp"
#include "erpc_mu_transport.hpp"
#include "erpc_transport_setup.h"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

ERPC_MANUALLY_CONSTRUCTED_STATIC(MUTransport, s_muTransport);

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_transport_t erpc_transport_mu_init(void *baseAddr)
{
    MUTransport *muTransport;

#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    if (s_muTransport.isUsed())
    {
        muTransport = NULL;
    }
    else
    {
        s_muTransport.construct();
        muTransport = s_muTransport.get();
    }
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    muTransport = new MUTransport();
#else
#error "Unknown eRPC allocation policy!"
#endif

    if (muTransport != NULL)
    {
        (void)muTransport->init(reinterpret_cast<MU_Type *>(baseAddr));
    }

    return reinterpret_cast<erpc_transport_t>(muTransport);
}

void erpc_transport_mu_deinit(erpc_transport_t transport)
{
#if ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_STATIC
    (void)transport;
    s_muTransport.destroy();
#elif ERPC_ALLOCATION_POLICY == ERPC_ALLOCATION_POLICY_DYNAMIC
    erpc_assert(transport != NULL);

    MUTransport *muTransport = reinterpret_cast<MUTransport *>(transport);

    delete muTransport;
#endif
}
