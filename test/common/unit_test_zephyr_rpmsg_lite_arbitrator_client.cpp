/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <zephyr/kernel.h>

#include "erpc_arbitrated_client_setup.h"
#include "erpc_mbf_setup.h"
#include "erpc_server_setup.h"
#include "erpc_transport_setup.h"
#include "erpc_threading.h"

// #include "FreeRTOS.h"
// #include "semphr.h"
// #include "task.h"

#include "c_test_firstInterface_client.h"
#include "c_test_secondInterface_server.h"
#include "gtest.h"
#include "unit_test.h"
#include "unit_test_wrapped.h"

#include "rpmsg_lite.h"

// #ifdef UNITY_DUMP_RESULTS
// #include "corn_g_test.h"
// #endif

using namespace erpc;
using namespace std;

#define APP_ERPC_READY_EVENT_DATA (1)
#define RPMSG_LITE_LINK_ID (0)

Mutex waitQuitMutex;
Thread g_initThread("runInit");
Thread g_serverThread("runServer");
Thread g_clientThread("runClient");

K_THREAD_STACK_DEFINE(stack_init, 256 * 4);
K_THREAD_STACK_DEFINE(stack_server, 1536 * 8);
K_THREAD_STACK_DEFINE(stack_client, 1536 * 8);

volatile int waitQuit = 0;
volatile uint16_t eRPCReadyEventData = 0;
extern const uint32_t erpc_generated_crc;
erpc_service_t service = NULL;
erpc_server_t server;

struct k_poll_signal signalClient;
struct k_poll_signal signalServer;

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
    struct k_poll_event events[1] = {
        K_POLL_EVENT_INITIALIZER(K_POLL_TYPE_SIGNAL, K_POLL_MODE_NOTIFY_ONLY, &signalServer)
    };
    k_poll(events, 1, K_FOREVER);

    erpc_status_t err;
    err = erpc_server_run(server);
    increaseWaitQuit();

    if (err != kErpcStatus_Success)
    {
        printk("Server error: %d\r\n", err);
    }

    k_thread_suspend(k_current_get());
}

void runClient(void *arg)
{
    struct k_poll_event events[1] = {
        K_POLL_EVENT_INITIALIZER(K_POLL_TYPE_SIGNAL, K_POLL_MODE_NOTIFY_ONLY, &signalClient),
    };
    k_poll(events, 1, K_FOREVER);

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

    k_thread_suspend(k_current_get());
}

void runInit(void *arg)
{
    erpc_transport_t transportClient;
    erpc_transport_t transportServer;

    // RPMsg-Lite transport layer initialization
    transportClient = erpc_transport_rpmsg_lite_rtos_master_init(100, 101, RPMSG_LITE_LINK_ID);

    if (transportClient == NULL)
    {
        // error in initialization of transport layer
        printk("Transport layer initialization failed\r\n");
        while (1)
        {
        }
    }

    // MessageBufferFactory initialization
    erpc_mbf_t message_buffer_factory;
    erpc_client_t client;

    message_buffer_factory = erpc_mbf_rpmsg_init(transportClient);

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

    k_poll_signal_raise(&signalClient, 0);
    k_poll_signal_raise(&signalServer, 0);

    k_thread_suspend(k_current_get());
}

class ZephyrPrinter : public ::testing::EmptyTestEventListener
{
    virtual void OnTestCaseStart(const ::testing::TestCase &test_case)
    {
        printk("[----------] %d %s from  %s\n", test_case.test_to_run_count(),
               (test_case.test_to_run_count() == 1 ? "test" : "tests"), test_case.name());
    }

    virtual void OnTestStart(const ::testing::TestInfo &test_info)
    {
        printk("[ RUN      ] %s.%s\n", test_info.test_case_name(), test_info.name());
    }

    virtual void OnTestDisabled(const ::testing::TestInfo &test_info)
    {
        printk("[ DISABLED ] %s.%s\n", test_info.test_case_name(), test_info.name());
    }

    virtual void OnTestPartResult(const ::testing::TestPartResult &test_part_result)
    {
        if (test_part_result.type() == ::testing::TestPartResult::kSuccess)
        {
            return;
        }
        else
        {
            printk("%s (%d) %s\n%s\n", test_part_result.file_name(), test_part_result.line_number(),
                   test_part_result.failed() ? "error" : "Success", test_part_result.summary());
        }
    }

    virtual void OnTestEnd(const ::testing::TestInfo &test_info)
    {
        if (test_info.result()->Passed())
        {
            printk("[       OK ] ");
        }
        else
        {
            printk("[  FAILED  ] ");
        }
        printk("%s.%s\n", test_info.test_case_name(), test_info.name());
    }

    virtual void OnTestCaseEnd(const ::testing::TestCase &test_case)
    {
        printk("%d %s from %s\n", test_case.test_to_run_count(),
               (test_case.test_to_run_count() == 1 ? "test" : "tests"), test_case.name());
    }

    virtual void OnTestProgramEnd(const ::testing::UnitTest &) { printk("[==========] Done running all tests.\r\n"); }
};

int main(void)
{
    env_sleep_msec(1000);
    int fake_argc = 1;
    const auto fake_arg0 = "dummy";
    char *fake_argv0 = const_cast<char *>(fake_arg0);
    char **fake_argv = &fake_argv0;
    ::testing::InitGoogleTest(&fake_argc, fake_argv);

    ::testing::TestEventListeners &listeners = ::testing::UnitTest::GetInstance()->listeners();
    delete listeners.Release(listeners.default_result_printer());
    listeners.Append(new ZephyrPrinter);

    k_poll_signal_init(&signalClient);
    k_poll_signal_init(&signalServer);

    g_initThread.init(&runInit, 1, 256 * 4);
    g_serverThread.init(&runServer, 2, 1536 * 4);
    g_clientThread.init(&runClient, 1, 1536 * 4);

    g_initThread.setStackPointer(stack_init);
    g_serverThread.setStackPointer(stack_server);
    g_clientThread.setStackPointer(stack_client);

    g_initThread.start();
    g_serverThread.start();
    g_clientThread.start();

    k_thread_suspend(k_current_get());
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
