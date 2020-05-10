/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__UNIT_TEST_H_
#define _EMBEDDED_RPC__UNIT_TEST_H_

#include "erpc_simple_server.h"

////////////////////////////////////////////////////////////////////////////////
// Function Prototypes
////////////////////////////////////////////////////////////////////////////////

void add_services(erpc::SimpleServer *server);

void add_common_service(erpc::SimpleServer *server);

#endif // _EMBEDDED_RPC__UNIT_TEST_H_
