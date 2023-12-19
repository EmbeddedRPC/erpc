/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <zephyr/kernel.h>

#include "rpmsg_lite.h"
#include "erpc_client_setup.h"
#include "erpc_matrix_multiply_common.h"
#include "c_erpc_matrix_multiply_client.h"
#include "erpc_error_handler.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define ERPC_TRANSPORT_RPMSG_LITE_LINK_ID (RL_PLATFORM_IMXRT1160_M7_M4_LINK_ID)

extern char rpmsg_lite_base[];

#define MATRIX_ITEM_MAX_VALUE     (50)
#define APP_ERPC_READY_EVENT_DATA (1U)


/*******************************************************************************
 * Variables
 ******************************************************************************/
extern bool g_erpc_error_occurred;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Fill matrices by random values
 */
static void fill_matrices(Matrix matrix1_ptr, Matrix matrix2_ptr)
{
    int32_t a, b;

    /* Fill both matrices by random values */
    for (a = 0; a < matrix_size; ++a)
    {
        for (b = 0; b < matrix_size; ++b)
        {
            matrix1_ptr[a][b] = rand() % MATRIX_ITEM_MAX_VALUE;
            matrix2_ptr[a][b] = rand() % MATRIX_ITEM_MAX_VALUE;
        }
    }
}

/*!
 * @brief Printing a matrix to the console
 */
static void print_matrix(Matrix matrix_ptr)
{
    int32_t a, b;

    for (a = 0; a < matrix_size; ++a)
    {
        for (b = 0; b < matrix_size; ++b)
        {
            printk("%4i ", matrix_ptr[a][b]);
        }
        printk("\r\n");
    }
}

int main(void)
{
    Matrix matrix1 = {0}, matrix2 = {0}, result_matrix = {0};

    /* RPMsg-Lite transport layer initialization */
    erpc_transport_t transport;
    erpc_client_t client;

    transport = erpc_transport_rpmsg_lite_rtos_master_init(100, 101, ERPC_TRANSPORT_RPMSG_LITE_LINK_ID);

    /* MessageBufferFactory initialization */
    erpc_mbf_t message_buffer_factory;
    message_buffer_factory = erpc_mbf_rpmsg_init(transport);

    /* eRPC client side initialization */
    client = erpc_client_init(transport, message_buffer_factory);
		initMatrixMultiplyService_client(client);

    /* Set default error handler */
    erpc_client_set_error_handler(client, erpc_error_handler);

    /* Fill both matrices by random values */
    fill_matrices(matrix1, matrix2);

    /* Print both matrices on the console */
    printk("\r\nMatrix #1");
    printk("\r\n=========\r\n");
    print_matrix(matrix1);

    printk("\r\nMatrix #2");
    printk("\r\n=========\r\n");
    print_matrix(matrix2);

    for (;;)
    {
        printk("\r\neRPC request is sent to the server\r\n");

        erpcMatrixMultiply(matrix1, matrix2, result_matrix);

        /* Check if some error occurred in eRPC */
        if (g_erpc_error_occurred)
        {
            /* Exit program loop */
            break;
        }

        printk("\r\nResult matrix");
        printk("\r\n=============\r\n");
        print_matrix(result_matrix);

        env_sleep_msec(1000);

        /* Wait for 200ms to eliminate the button glitch */
        env_sleep_msec(200);

        /* Fill both matrices by random values */
        fill_matrices(matrix1, matrix2);

        /* Print both matrices on the console */
        printk("\r\nMatrix #1");
        printk("\r\n=========\r\n");
        print_matrix(matrix1);

        printk("\r\nMatrix #2");
        printk("\r\n=========\r\n");
        print_matrix(matrix2);
    }
    for (;;)
    {
    }
}
