/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "unit_test.h"

#include "erpc_arbitrated_client_setup.h"
#include "erpc_mbf_setup.h"
#include "erpc_server_setup.h"
#include "erpc_transport_setup.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "gtest.h"

#include "test_firstInterface.h"
#include "test_secondInterface_server.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "app_core0.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "mcmgr.h"
#include "rpmsg_lite.h"
#ifdef __cplusplus
}
#endif

#ifdef UNITY_DUMP_RESULTS
#include "corn_g_test.h"
#endif

using namespace erpc;
using namespace std;

#define APP_ERPC_READY_EVENT_DATA (1)

SemaphoreHandle_t g_waitQuitMutex;
TaskHandle_t g_serverTask;
TaskHandle_t g_clientTask;
volatile int waitQuit = 0;
volatile uint16_t eRPCReadyEventData = 0;
extern const uint32_t erpc_generated_crc;

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
    xSemaphoreTake(g_waitQuitMutex, portMAX_DELAY);
    waitQuit++;
    xSemaphoreGive(g_waitQuitMutex);
}

void runServer(void *arg)
{
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    erpc_status_t err;
    err = erpc_server_run();
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

    RUN_ALL_TESTS();

    // wait until ERPC second (server) app will announce ready to quit state.
    while (true)
    {
        if (waitQuit != 0)
        {
            break;
        }
        vTaskDelay(10);
    }

    // send to ERPC second (server) app ready to quit state
    quitFirstInterfaceServer();

    increaseWaitQuit();

    vTaskSuspend(NULL);
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

void runInit(void *arg)
{
    // Initialize MCMGR before calling its API
    MCMGR_Init();

    /* Register the application event before starting the secondary core */
    MCMGR_RegisterEvent(kMCMGR_RemoteApplicationEvent, eRPCReadyEventHandler, NULL);

    // Boot source for Core 1
    MCMGR_StartCore(kMCMGR_Core1, CORE1_BOOT_ADDRESS, (uint32_t)rpmsg_lite_base, kMCMGR_Start_Synchronous);

    // RPMsg-Lite transport layer initialization
    erpc_transport_t transportClient;
    erpc_transport_t transportServer;
    transportClient = erpc_transport_rpmsg_lite_rtos_master_init(100, 101, ERPC_TRANSPORT_RPMSG_LITE_LINK_ID);
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
    message_buffer_factory = erpc_mbf_rpmsg_init(transportClient);

    // eRPC client side initialization
    transportServer = erpc_arbitrated_client_init(transportClient, message_buffer_factory);

    // eRPC server side initialization
    erpc_server_t server = erpc_server_init(transportServer, message_buffer_factory);

    erpc_arbitrated_client_set_crc(erpc_generated_crc);

    // adding server to client for nested calls.
    erpc_arbitrated_client_set_server(server);
    erpc_arbitrated_client_set_server_thread_id((void *)g_serverTask);

    // adding the service to the server
    erpc_add_service_to_server(create_SecondInterface_service());

    // unblock server and client task
    xTaskNotifyGive(g_serverTask);
    xTaskNotifyGive(g_clientTask);

    // Wait until server and client will stop.
    while (true)
    {
        if (waitQuit >= 3)
        {
            break;
        }
        vTaskDelay(500);
    }

    vTaskSuspend(NULL);
}

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
        PRINTF("%s in %s:%d\r\n%s\r\n", test_part_result.failed() ? "*** Failure" : "Success", test_part_result.file_name(),
               test_part_result.line_number(), test_part_result.summary());
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

#ifdef __cplusplus
extern "C" {
#endif
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
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
    PRINTF("Copy CORE1 image to address: 0x%x, size: %d\r\n", CORE1_BOOT_ADDRESS, core1_image_size);

    // Copy application from FLASH to RAM
    memcpy(CORE1_BOOT_ADDRESS, (void *)CORE1_IMAGE_START, core1_image_size);
#endif

    g_waitQuitMutex = xSemaphoreCreateMutex();
    xTaskCreate(runInit, "runInit", 256, NULL, 1, NULL);
    xTaskCreate(runServer, "runServer", 1536, NULL, 2, &g_serverTask);
    xTaskCreate(runClient, "runClient", 1536, NULL, 1, &g_clientTask);

    vTaskStartScheduler();

    while (1)
    {
    }
}
#ifdef __cplusplus
}
#endif

void quitSecondInterfaceServer()
{
    // Stop server part
    erpc_server_stop();
    increaseWaitQuit();
}
