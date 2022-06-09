/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_client_setup.h"
#include "erpc_mbf_setup.h"
#include "erpc_transport_setup.h"

#if (defined(RPMSG) || defined(UART) || defined(MU))
extern "C" {
#if defined(RPMSG)
#include "rpmsg_lite.h"
#elif defined(UART)
#include "fsl_usart_cmsis.h"
#endif
#include "app_core0.h"
#include "fsl_debug_console.h"
#include "mcmgr.h"
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
int main(int argc, char **argv);
#endif
}

#include "board.h"
#include "gtest.h"
#include "gtestListener.hpp"
#include "myAlloc.hpp"
#include "test_unit_test_common.h"

#ifdef UNITY_DUMP_RESULTS
#include "corn_g_test.h"
#endif

using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

/************************************************************************************
 * Following snippet reused from https://github.com/google/googletest/blob/master/googletest/docs/advanced.md
 * Copyright 2008, Google Inc.
 * SPDX-License-Identifier: BSD-3-Clause
 */

class MinimalistPrinter : public ::testing::EmptyTestEventListener
{
    // Called before a test starts.
    virtual void OnTestStart(const ::testing::TestInfo &test_info)
    {
        PRINTF("*** Test %s.%s starting.\r\n", test_info.test_case_name(), test_info.name());
    }

    // Called after a failed assertion or a SUCCEED() invocation.
    virtual void OnTestPartResult(const ::testing::TestPartResult &test_part_result)
    {
        PRINTF("%s in %s:%d\r\n%s\r\n", test_part_result.failed() ? "*** Failure" : "Success",
               test_part_result.file_name(), test_part_result.line_number(), test_part_result.summary());
    }

    // Called after a test ends.
    virtual void OnTestEnd(const ::testing::TestInfo &test_info)
    {
        PRINTF("*** Test %s.%s ending.\r\n", test_info.test_case_name(), test_info.name());
    }

    virtual void OnTestCaseEnd(const ::testing::TestCase &test_case)
    {
        PRINTF("*** Total tests passed: %d, failed: %d.\r\n", test_case.successful_test_count(),
               test_case.failed_test_count());
    }
};
/*
 * end of reused snippet
 ***********************************************************************************/
#endif

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

int MyAlloc::allocated_ = 0;

#if defined(RPMSG)
#define APP_ERPC_READY_EVENT_DATA (1)
extern char rpmsg_lite_base[];
volatile uint16_t eRPCReadyEventData = 0;
#elif defined(MU)
#define APP_ERPC_READY_EVENT_DATA (1)
volatile uint16_t eRPCReadyEventData = 0;
#endif

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////
#if (defined(RPMSG) || defined(MU))
/*!
 * @brief eRPC server side ready event handler
 */
static void eRPCReadyEventHandler(uint16_t eventData, void *context)
{
    eRPCReadyEventData = eventData;
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

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    ::testing::TestEventListeners &listeners = ::testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new LeakChecker);

#if (defined(RPMSG) || defined(UART) || defined(MU))
    delete listeners.Release(listeners.default_result_printer());
    listeners.Append(new MinimalistPrinter);
#ifdef UNITY_DUMP_RESULTS
    listeners.Append(new CornTestingFrameworkPrint());
#endif

    /* Initialize GIC */
    BOARD_InitHardware();

#ifdef CORE1_IMAGE_COPY_TO_RAM
    /* Calculate size of the image */
    uint32_t core1_image_size;
    core1_image_size = get_core1_image_size();
    PRINTF("Copy CORE1 image to address: 0x%x, size: %d\r\n", (void *)(char *)CORE1_BOOT_ADDRESS, core1_image_size);

    /* Copy application from FLASH to RAM */
    memcpy((void *)(char *)CORE1_BOOT_ADDRESS, (void *)CORE1_IMAGE_START, core1_image_size);
#endif

#if defined(RPMSG)
    env_init();
#endif
#endif

#if defined(RPMSG)
    /* Initialize MCMGR before calling its API */
    MCMGR_Init();

    /* Register the application event before starting the secondary core */
    MCMGR_RegisterEvent(kMCMGR_RemoteApplicationEvent, eRPCReadyEventHandler, NULL);

    /* Boot Secondary core application */
    MCMGR_StartCore(kMCMGR_Core1, (void *)(char *)CORE1_BOOT_ADDRESS, (uint32_t)rpmsg_lite_base,
                    kMCMGR_Start_Synchronous);

    /* Wait until the secondary core application signals the rpmsg remote has been initialized and is ready to
     * communicate. */
    while (APP_ERPC_READY_EVENT_DATA != eRPCReadyEventData)
    {
    };
#elif defined(MU)
    /* Initialize MCMGR before calling its API */
    MCMGR_Init();

    /* Register the application event before starting the secondary core */
    MCMGR_RegisterEvent(kMCMGR_RemoteApplicationEvent, eRPCReadyEventHandler, NULL);

    /* Boot Secondary core application */
    MCMGR_StartCore(kMCMGR_Core1, (void *)(char *)CORE1_BOOT_ADDRESS, (uint32_t)0, kMCMGR_Start_Asynchronous);

    /* Wait until the secondary core application signals the rpmsg remote has been initialized and is ready to
     * communicate. */
    while (APP_ERPC_READY_EVENT_DATA != eRPCReadyEventData)
    {
    };
#endif

    erpc_transport_t transport;
    erpc_mbf_t message_buffer_factory;
#if defined(RPMSG)
    transport = erpc_transport_rpmsg_lite_master_init(100, 101, ERPC_TRANSPORT_RPMSG_LITE_LINK_ID);
    message_buffer_factory = erpc_mbf_rpmsg_init(transport);
#elif defined(UART)
    transport = erpc_transport_cmsis_uart_init((void *)&Driver_USART0);
    message_buffer_factory = erpc_mbf_dynamic_init();
#elif defined(MU)
    transport = erpc_transport_mu_init(MU_BASE);
    message_buffer_factory = erpc_mbf_dynamic_init();
#endif

    erpc_client_init(transport, message_buffer_factory);

    int i = RUN_ALL_TESTS();
    quit();
#if defined(RPMSG)
    /* wait a while to allow the erpc server side to finalize shutdown,
       otherwise an IPC interrupt can be triggered on the client side at the
       time the rpmsg is deinitilaized yet => hardfault */
    env_sleep_msec(10000);
#endif
    erpc_client_deinit();

    return i;
}
