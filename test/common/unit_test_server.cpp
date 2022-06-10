/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2020 NXP
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
#if defined(RPMSG)
#define APP_ERPC_READY_EVENT_DATA (1)
#include "mcmgr.h"
#include "rpmsg_lite.h"
#elif defined(UART)
#include "fsl_usart_cmsis.h"
#elif defined(MU)
#define APP_ERPC_READY_EVENT_DATA (1)
#include "mcmgr.h"
#endif
#include "app_core1.h"
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
int main(int argc, const char *argv[]);
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

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////
#if (defined(RPMSG) || defined(MU))
static void SignalReady(void)
{
    /* Signal the other core we are ready by trigerring the event and passing the APP_ERPC_READY_EVENT_DATA */
    MCMGR_TriggerEvent(kMCMGR_RemoteApplicationEvent, APP_ERPC_READY_EVENT_DATA);
}
/*!
 * @brief Application-specific implementation of the SystemInitHook() weak function.
 */
void SystemInitHook(void)
{
    /* Initialize MCMGR - low level multicore management library. Call this
       function as close to the reset entry as possible to allow CoreUp event
       triggering. The SystemInitHook() weak function overloading is used in this
       application. */
    MCMGR_EarlyInit();
}
#endif

int main(int argc, const char *argv[])
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
    erpc_server_init(transport, message_buffer_factory);

    /* Add test services. This function call erpc_add_service_to_server for all necessary services. */
    add_services_to_server();

    /* Add common service */
    add_common_service();

#if defined(MU)
    SignalReady();
#endif
    /* Add run server */
    erpc_server_run();

    /* Deinit server */
    erpc_server_deinit();

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Server helper functions
////////////////////////////////////////////////////////////////////////////////

void add_common_service()
{
    service_common = create_Common_service();
    erpc_add_service_to_server(service_common);
}

////////////////////////////////////////////////////////////////////////////////
// Common service implementations here
////////////////////////////////////////////////////////////////////////////////

void quit()
{
    /* removing common services from the server */
    remove_common_services_from_server(service_common);

    /* removing individual test services from the server */
    remove_services_from_server();

    erpc_server_stop();
}

int32_t getServerAllocated()
{
    int result = MyAlloc::allocated();
    MyAlloc::allocated(0);
    return result;
}
