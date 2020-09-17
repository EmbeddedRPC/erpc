/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _EMBEDDED_RPC__UNIT_TEST_WRAPPED_H_
#define _EMBEDDED_RPC__UNIT_TEST_WRAPPED_H_

////////////////////////////////////////////////////////////////////////////////
// Function Prototypes
////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif
void add_services_to_server();
void remove_services_from_server();
void remove_common_services_from_server(erpc_service_t service);
void add_common_service();
#ifdef __cplusplus
}
#endif

#endif // _EMBEDDED_RPC__UNIT_TEST_WRAPPED_H_
