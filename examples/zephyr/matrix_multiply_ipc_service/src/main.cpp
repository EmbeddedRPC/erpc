/*
 * Copyright 2023-2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <zephyr/kernel.h>

#include "erpc_client_setup.h"
#include "erpc_transport_setup.h"
#include "erpc_matrix_multiply_common.h"
#include "c_erpc_matrix_multiply_client.h"
#include "erpc_error_handler.h"

#if defined(CONFIG_BOARD_MIMXRT685_EVK) || defined(CONFIG_BOARD_MIMXRT700_EVK)
#include "dsp.h"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define ERPC_ENDPOINT_NAME "erpc_ept"
#define MASTER_INIT_DELAY_MS (500U)

#define MATRIX_ITEM_MAX_VALUE (50)

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
    Matrix matrix1 = { 0 }, matrix2 = { 0 }, result_matrix = { 0 };

    printk("\r\n");
    printk("*************************************************\r\n");
    printk("* eRPC Matrix Multiply Demo - Master/Host Core *\r\n");
    printk("*************************************************\r\n");
    printk("\r\n");

#if defined(CONFIG_BOARD_MIMXRT685_EVK) || defined(CONFIG_BOARD_MIMXRT700_EVK)
    /* Start DSP core */
    printk("Starting DSP core...\r\n");
    dsp_start();
    k_sleep(K_MSEC(500));
#endif

    /* Zephyr IPC Service transport layer initialization */
    printk("Initializing Zephyr IPC Service transport...\r\n");
    erpc_transport_t transport = erpc_transport_zephyr_ipc_master_init(ERPC_ENDPOINT_NAME, MASTER_INIT_DELAY_MS);

    if (transport == NULL)
    {
        printk("ERROR: Failed to initialize eRPC transport\r\n");
        return -1;
    }

    printk("Transport initialized successfully\r\n");

    /* MessageBufferFactory initialization */
    erpc_mbf_t message_buffer_factory;
    message_buffer_factory = erpc_mbf_ipc_service_init(transport);

    /* eRPC client side initialization */
    erpc_client_t client = erpc_client_init(transport, message_buffer_factory);
    initMatrixMultiplyService_client(client);

    /* Set default error handler */
    erpc_client_set_error_handler(client, erpc_error_handler);

    printk("eRPC client initialized\r\n");
    printk("\r\n");

    /* Fill both matrices by random values */
    fill_matrices(matrix1, matrix2);

    /* Print both matrices on the console */
    printk("Matrix #1\r\n");
    printk("=========\r\n");
    print_matrix(matrix1);

    printk("\r\nMatrix #2\r\n");
    printk("=========\r\n");
    print_matrix(matrix2);

    for (;;)
    {
        printk("\r\nSending eRPC request to remote core...\r\n");

        /* Call remote matrix multiply function */
        erpcMatrixMultiply(matrix1, matrix2, result_matrix);

        /* Check if some error occurred in eRPC */
        if (g_erpc_error_occurred)
        {
            printk("ERROR: eRPC error occurred, exiting...\r\n");
            break;
        }

        printk("Received result from remote core\r\n");
        printk("\r\nResult matrix\r\n");
        printk("=============\r\n");
        print_matrix(result_matrix);

        /* Wait before next iteration */
        k_sleep(K_MSEC(1000));

        /* Fill both matrices by random values for next iteration */
        fill_matrices(matrix1, matrix2);

        /* Print both matrices on the console */
        printk("\r\nMatrix #1\r\n");
        printk("=========\r\n");
        print_matrix(matrix1);

        printk("\r\nMatrix #2\r\n");
        printk("=========\r\n");
        print_matrix(matrix2);
    }

    /* Cleanup on error */
    printk("Cleaning up...\r\n");
    erpc_client_deinit(client);
    erpc_mbf_dynamic_deinit(message_buffer_factory);
    erpc_transport_zephyr_ipc_master_deinit(transport);

    for (;;)
    {
        k_sleep(K_FOREVER);
    }

    return 0;
}
