/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2024 NXP
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

#include "c_test_firstInterface_server.h"
#include "c_test_secondInterface_client.h"
#include "unit_test.h"
#include "unit_test_wrapped.h"

using namespace erpc;

#define UART_DEVICE_NODE DT_CHOSEN(zephyr_console)
static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

int testClient();

Mutex waitQuitMutex;
Thread g_initThread("runInit");
Thread g_serverThread("runServer");
Thread g_clientThread("runClient");

volatile int waitQuit = 0;
volatile int waitClient = 0;
volatile int isTestPassing = 0;
uint32_t startupData;
volatile int stopTest = 0;
extern const uint32_t erpc_generated_crc;
erpc_service_t service = NULL;
erpc_server_t server;

K_THREAD_STACK_DEFINE(stack_init, 256 * 4);
K_THREAD_STACK_DEFINE(stack_server, 1536 * 4);
K_THREAD_STACK_DEFINE(stack_client, 1536 * 4);

struct k_poll_signal signalClient;
struct k_poll_signal signalServer;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////
void increaseWaitQuit()
{
    Mutex::Guard lock(waitQuitMutex);
    waitQuit++;
}

void runServer(void *arg)
{
    struct k_poll_event events[1] = {
        K_POLL_EVENT_INITIALIZER(K_POLL_TYPE_SIGNAL, K_POLL_MODE_NOTIFY_ONLY, &signalServer),
    };
    k_poll(events, 1, K_FOREVER);

    erpc_status_t err;
    err = erpc_server_run(server);
    increaseWaitQuit();

    if (err != kErpcStatus_Success)
    {
        // server error
        while (1)
        {
        }
    }
    k_thread_suspend(k_current_get());
}

void runClient(void *arg)
{
    struct k_poll_event events[1] = {
        K_POLL_EVENT_INITIALIZER(K_POLL_TYPE_SIGNAL, K_POLL_MODE_NOTIFY_ONLY, &signalClient),
    };
    k_poll(events, 1, K_FOREVER);

    // wait until ERPC first (client) app will announce that it is ready.
    while (waitClient == 0)
    {
        Thread::sleep(10000);
    }

    // wait until ERPC first (client) app will announce ready to quit state
    while (true)
    {
        isTestPassing = testClient();
        {
            Thread::sleep(10000);
            Mutex::Guard lock(waitQuitMutex);
            if (waitQuit != 0 || isTestPassing != 0 || stopTest != 0)
            {
                enableFirstSide();
                break;
            }
        }
    }

    while (true)
    {
        Thread::sleep(100000);
        Mutex::Guard lock(waitQuitMutex);
        if (waitQuit != 0)
        {
            break;
        }
    }

    // send to ERPC first (client) app ready to quit state
    quitSecondInterfaceServer();
    increaseWaitQuit();
    k_thread_suspend(k_current_get());
}

static void SignalReady(void) {}

void runInit(void *arg)
{
    if (!device_is_ready(uart_dev))
    {
        printk("UART device not found!");
        return;
    }

    erpc_transport_t transportClient;
    erpc_transport_t transportServer;
    erpc_mbf_t message_buffer_factory;
    erpc_client_t client;

    transportClient = erpc_transport_zephyr_uart_init((void *)uart_dev);

    // MessageBufferFactory initialization
    message_buffer_factory = erpc_mbf_dynamic_init();

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
    service = create_FirstInterface_service();
    erpc_add_service_to_server(server, service);

    // unblock server and client task
    k_poll_signal_raise(&signalClient, 0);
    k_poll_signal_raise(&signalServer, 0);

    k_thread_suspend(k_current_get());
}

int main(void)
{
    k_poll_signal_init(&signalClient);
    k_poll_signal_init(&signalServer);

    g_initThread.init(&runInit, 1, 256 * 4);
    g_serverThread.init(&runServer, 3, 1536 * 4);
    g_clientThread.init(&runClient, 2, 1536 * 4);

    g_initThread.setStackPointer(stack_init);
    g_serverThread.setStackPointer(stack_server);
    g_clientThread.setStackPointer(stack_client);


    g_initThread.start();
    g_serverThread.start();
    g_clientThread.start();

    k_thread_suspend(k_current_get());
}

extern "C" {
void stopSecondSide()
{
    ++stopTest;
}

int32_t getResultFromSecondSide()
{
    return isTestPassing;
}

void testCasesAreDone(void)
{
    increaseWaitQuit();
}

void quitFirstInterfaceServer()
{
    /* removing the service from the server */
    erpc_remove_service_from_server(server, service);
    destroy_FirstInterface_service(service);

    // Stop server part
    erpc_server_stop(server);
}

void whenReady()
{
    waitClient++;
}
}

int testClient()
{
    int number = 15;
    for (int i = 0; i < number; i++)
    {
        secondSendInt(i + number);
    }
    for (int i = number - 1; i >= 0; i--)
    {
        if (i + number != secondReceiveInt())
        {
            return -1;
        }
    }
    return 0;
}
