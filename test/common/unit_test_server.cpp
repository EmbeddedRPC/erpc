/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_mbf_setup.h"
#include "erpc_server_setup.h"
#include "erpc_simple_server.hpp"
#include "erpc_transport_setup.h"

#if (defined(RPMSG) || defined(UART) || defined(MU))
extern "C" {
#if defined(UART)
#include "fsl_lpuart_cmsis.h"
#include "app_core0.h"
#else
#if defined(RPMSG)
#include "rpmsg_lite.h"
#endif
#define APP_ERPC_READY_EVENT_DATA (1)
#include "mcmgr.h"
#include "app_core1.h"
#endif
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
int main(void);
#endif
}
#endif

#include "board.h"
#include "myAlloc.hpp"
#include "test_unit_test_common_server.h"
#include "unit_test_wrapped.h"

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

int MyAlloc::allocated_ = 0;
erpc_service_t service_common = NULL;
erpc_server_t server;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////
#if (defined(RPMSG) || defined(MU))
static void SignalReady(void)
{
    /* Signal the other core we are ready by trigerring the event and passing the APP_ERPC_READY_EVENT_DATA */
    MCMGR_TriggerEvent(kMCMGR_RemoteApplicationEvent, APP_ERPC_READY_EVENT_DATA);
}
#endif

int main(void)
{
    BOARD_InitHardware();

#if (defined(RPMSG) || defined(MU))
    uint32_t startupData;
    mcmgr_status_t status;

    /* Initialize MCMGR before calling its API */
    MCMGR_Init();
    /* Get the startup data */
    do
    {
        status = MCMGR_GetStartupData(&startupData);
    } while (status != kStatus_MCMGR_Success);
#endif

    erpc_transport_t transport;
    erpc_mbf_t message_buffer_factory;

#if defined(RPMSG)
    transport = erpc_transport_rpmsg_lite_remote_init(101, 100, (void *)startupData, ERPC_TRANSPORT_RPMSG_LITE_LINK_ID,
                                                      SignalReady, NULL);
    message_buffer_factory = erpc_mbf_rpmsg_init(transport);
#elif defined(UART)
    transport = erpc_transport_cmsis_uart_init((void *)&Driver_USART0);
    message_buffer_factory = erpc_mbf_dynamic_init();
#elif defined(MU)
    transport = erpc_transport_mu_init(MU_BASE);
    message_buffer_factory = erpc_mbf_dynamic_init();
#endif

    /* Init server */
    server = erpc_server_init(transport, message_buffer_factory);

    /* Add test services. This function call erpc_add_service_to_server for all necessary services. */
    add_services_to_server(server);

    /* Add common service */
    add_common_service(server);

#if defined(MU)
    SignalReady();
#endif
    /* Add run server */
    erpc_server_run(server);

    /* Deinit server */
    erpc_server_deinit(server);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Server helper functions
////////////////////////////////////////////////////////////////////////////////

void add_common_service(erpc_server_t server)
{
    service_common = create_Common_service();
    erpc_add_service_to_server(server, service_common);
}

////////////////////////////////////////////////////////////////////////////////
// Common service implementations here
////////////////////////////////////////////////////////////////////////////////

void quit()
{
    /* removing common services from the server */
    remove_common_services_from_server(server, service_common);

    /* removing individual test services from the server */
    remove_services_from_server(server);

    erpc_server_stop(server);
}

int32_t getServerAllocated()
{
    int result = MyAlloc::allocated();
    MyAlloc::allocated(0);
    return result;
}
