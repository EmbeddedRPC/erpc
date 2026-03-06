/*
 * Copyright 2023-2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <zephyr/kernel.h>

#include "erpc_server_setup.h"
#include "erpc_transport_setup.h"
#include "erpc_matrix_multiply_common.h"
#include "c_erpc_matrix_multiply_server.h"
#include "erpc_error_handler.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define ERPC_ENDPOINT_NAME "erpc_ept"
#define REMOTE_INIT_DELAY_MS (500U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief erpcMatrixMultiply function implementation.
 *
 * This is the implementation of the erpcMatrixMultiply function called by the primary core.
 *
 * @param matrix1 First matrix
 * @param matrix2 Second matrix
 * @param result_matrix Result matrix
 */
void erpcMatrixMultiply(Matrix matrix1, Matrix matrix2, Matrix result_matrix)
{
    int32_t i, j, k;

    /* Clear the result matrix */
    for (i = 0; i < matrix_size; ++i)
    {
        for (j = 0; j < matrix_size; ++j)
        {
            result_matrix[i][j] = 0;
        }
    }

    /* Multiply two matrices */
    for (i = 0; i < matrix_size; ++i)
    {
        for (j = 0; j < matrix_size; ++j)
        {
            for (k = 0; k < matrix_size; ++k)
            {
                result_matrix[i][j] += matrix1[i][k] * matrix2[k][j];
            }
        }
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    erpc_transport_t transport;
    erpc_server_t server;
    erpc_mbf_t message_buffer_factory;
    erpc_service_t service;

    printk("\r\n");
    printk("**************************************************\r\n");
    printk("* eRPC Matrix Multiply Demo - Remote/Slave Core *\r\n");
    printk("**************************************************\r\n");
    printk("\r\n");

    /* Zephyr IPC Service transport layer initialization */
    printk("Initializing Zephyr IPC Service transport...\r\n");
    transport = erpc_transport_zephyr_ipc_remote_init(ERPC_ENDPOINT_NAME, 0);

    if (transport == NULL)
    {
        printk("ERROR: Failed to initialize eRPC transport\r\n");
        return -1;
    }

    printk("Transport initialized successfully\r\n");

    /* MessageBufferFactory initialization - use DYNAMIC, not RPMsg! */
    message_buffer_factory = erpc_mbf_ipc_service_init(transport);
    if (message_buffer_factory == NULL)
    {
        printk("ERROR: Failed to initialize message buffer factory\r\n");
        erpc_transport_zephyr_ipc_remote_deinit(transport);
        return -1;
    }

    /* eRPC server side initialization */
    server = erpc_server_init(transport, message_buffer_factory);
    if (server == NULL)
    {
        printk("ERROR: Failed to initialize eRPC server\r\n");
        erpc_mbf_dynamic_deinit(message_buffer_factory);
        erpc_transport_zephyr_ipc_remote_deinit(transport);
        return -1;
    }

    /* Add the service to the server */
    service = create_MatrixMultiplyService_service();
    if (service == NULL)
    {
        printk("ERROR: Failed to create service\r\n");
        erpc_server_deinit(server);
        erpc_mbf_dynamic_deinit(message_buffer_factory);
        erpc_transport_zephyr_ipc_remote_deinit(transport);
        return -1;
    }

    erpc_add_service_to_server(server, service);

    printk("eRPC server initialized\r\n");
    printk("Ready to receive requests from master core\r\n");
    printk("\r\n");

    for (;;)
    {
        /* Process incoming eRPC messages */
        erpc_status_t status = erpc_server_poll(server);

        /* Handle error status */
        if (status != (erpc_status_t)kErpcStatus_Success)
        {
            /* Print error description */
            printk("ERROR: Server poll failed with status %d\r\n", status);
            erpc_error_handler(status, 0);

            /* Cleanup */
            printk("Cleaning up...\r\n");
            erpc_remove_service_from_server(server, service);
            destroy_MatrixMultiplyService_service(service);
            erpc_server_stop(server);
            erpc_server_deinit(server);
            erpc_mbf_dynamic_deinit(message_buffer_factory);
            erpc_transport_zephyr_ipc_remote_deinit(transport);

            /* Exit program loop */
            break;
        }

        /* Do other tasks while waiting for requests */
        k_yield();
    }

    /* Should not reach here */
    for (;;)
    {
        k_sleep(K_FOREVER);
    }

    return 0;
}
