/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_arbitrated_client_setup.h"
#include "erpc_mbf_setup.h"
#include "erpc_server_setup.h"
#include "erpc_transport_setup.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "c_test_firstInterface_client.h"
#include "c_test_secondInterface_server.h"
#include "gtest.h"
#include "unit_test.h"
#include "unit_test_wrapped.h"

#ifdef __cplusplus
extern "C" {
#endif
#if defined(RPMSG)
#include "rpmsg_lite.h"
#endif
#include "app_core0.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "mcmgr.h"
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
int main(void);
#endif
#ifdef __cplusplus
}
#endif

#ifdef UNITY_DUMP_RESULTS
#include "corn_g_test.h"
#endif

using namespace erpc;
using namespace std;

#define APP_ERPC_READY_EVENT_DATA (1)

Mutex waitQuitMutex;
Thread g_initThread("runInit");
Thread g_serverThread("runServer");
Thread g_clientThread("runClient");

volatile int waitQuit = 0;
volatile uint16_t eRPCReadyEventData = 0;
extern const uint32_t erpc_generated_crc;
erpc_service_t service = NULL;
erpc_server_t server;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////
/*!
 * @brief eRPC server side ready event handler
 */
static void eRPCReadyEventHandler(uint16_t eventData, void *context)
{
    eRPCReadyEventData = eventData;
}

void increaseWaitQuit()
{
    Mutex::Guard lock(waitQuitMutex);
    waitQuit++;
}

void runServer(void *arg)
{
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    erpc_status_t err;
    err = erpc_server_run(server);
    increaseWaitQuit();

    if (err != kErpcStatus_Success)
    {
        PRINTF("Server error: %d\r\n", err);
    }
    vTaskSuspend(NULL);
}

void runClient(void *arg)
{
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    // send to ERPC second (server) app message that this app is ready.
    whenReady();

    __attribute__((unused)) int i;
    i = RUN_ALL_TESTS();

    // wait until ERPC second (server) app will announce ready to quit state.
    while (true)
    {
        Mutex::Guard lock(waitQuitMutex);
        if (waitQuit != 0)
        {
            break;
        }
    }

    // send to ERPC second (server) app ready to quit state
    quitFirstInterfaceServer();

    increaseWaitQuit();

    vTaskSuspend(NULL);
}

void runInit(void *arg)
{
    // Initialize MCMGR before calling its API
    MCMGR_Init();

    /* Register the application event before starting the secondary core */
    MCMGR_RegisterEvent(kMCMGR_RemoteApplicationEvent, eRPCReadyEventHandler, NULL);

    // Boot source for Core 1
#if defined(RPMSG)
    MCMGR_StartCore(kMCMGR_Core1, (void *)(char *)CORE1_BOOT_ADDRESS, (uint32_t)rpmsg_lite_base,
                    kMCMGR_Start_Synchronous);
#elif defined(MU)
    MCMGR_StartCore(kMCMGR_Core1, (void *)(char *)CORE1_BOOT_ADDRESS, (uint32_t)0, kMCMGR_Start_Asynchronous);
#endif

    erpc_transport_t transportClient;
    erpc_transport_t transportServer;
#if defined(RPMSG)
    // RPMsg-Lite transport layer initialization
    transportClient = erpc_transport_rpmsg_lite_rtos_master_init(100, 101, ERPC_TRANSPORT_RPMSG_LITE_LINK_ID);
#elif defined(MU)
    transportClient = erpc_transport_mu_init(MU_BASE);
#endif
    if (transportClient == NULL)
    {
        // error in initialization of transport layer
        PRINTF("Transport layer initialization failed\r\n");
        while (1)
        {
        }
    }

    /* Wait until the secondary core application signals the rpmsg remote has been initialized and is ready to
     * communicate. */
    while (APP_ERPC_READY_EVENT_DATA != eRPCReadyEventData)
    {
    };

    // MessageBufferFactory initialization
    erpc_mbf_t message_buffer_factory;
    erpc_client_t client;
#if defined(RPMSG)
    message_buffer_factory = erpc_mbf_rpmsg_init(transportClient);
#elif defined(MU)
    message_buffer_factory = erpc_mbf_dynamic_init();
#endif

    // eRPC client side initialization
    client = erpc_arbitrated_client_init(transportClient, message_buffer_factory, &transportServer);
    initInterfaces(client);

    // eRPC server side initialization
    server = erpc_server_init(transportServer, message_buffer_factory);

    erpc_arbitrated_client_set_crc(client, erpc_generated_crc);

    // adding server to client for nested calls.
    erpc_arbitrated_client_set_server(client, server);
    erpc_arbitrated_client_set_server_thread_id(client, (void *)g_serverThread.getThreadId());

    // adding the service to the server
    service = create_SecondInterface_service();
    erpc_add_service_to_server(server, service);

    // unblock server and client task
    xTaskNotifyGive((TaskHandle_t)g_serverThread.getThreadId());
    xTaskNotifyGive((TaskHandle_t)g_clientThread.getThreadId());

    vTaskSuspend(NULL);
}

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

int main(void)
{
    int fake_argc = 1;
    const auto fake_arg0 = "dummy";
    char *fake_argv0 = const_cast<char *>(fake_arg0);
    char **fake_argv = &fake_argv0;
    ::testing::InitGoogleTest(&fake_argc, fake_argv);
    BOARD_InitHardware();

    ::testing::TestEventListeners &listeners = ::testing::UnitTest::GetInstance()->listeners();
    delete listeners.Release(listeners.default_result_printer());
    listeners.Append(new MinimalistPrinter);
#ifdef UNITY_DUMP_RESULTS
    listeners.Append(new CornTestingFrameworkPrint());
#endif

#ifdef CORE1_IMAGE_COPY_TO_RAM
    // Calculate size of the image
    uint32_t core1_image_size;
    core1_image_size = get_core1_image_size();
    PRINTF("Copy CORE1 image to address: 0x%x, size: %d\r\n", (void *)(char *)CORE1_BOOT_ADDRESS, core1_image_size);

    // Copy application from FLASH to RAM
    memcpy((void *)(char *)CORE1_BOOT_ADDRESS, (void *)CORE1_IMAGE_START, core1_image_size);
#endif

    g_initThread.init(&runInit, 1, 256 * 4);
    g_serverThread.init(&runServer, 2, 1536 * 4);
    g_clientThread.init(&runClient, 1, 1536 * 4);

    g_initThread.start();
    g_serverThread.start();
    g_clientThread.start();

    vTaskStartScheduler();

    while (1)
    {
    }
}

extern "C" {
void quitSecondInterfaceServer()
{
    /* removing the service from the server */
    erpc_remove_service_from_server(server, service);
    destroy_SecondInterface_service(service);

    // Stop server part
    erpc_server_stop(server);
    increaseWaitQuit();
}
}
