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

ERPC_MANUALLY_CONSTRUCTED(MUTransport, s_transport);

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_transport_t erpc_transport_mu_init(void *baseAddr)
{
    s_transport.construct();
    (void)s_transport->init((MU_Type *)baseAddr);
    return reinterpret_cast<erpc_transport_t>(s_transport.get());
}
