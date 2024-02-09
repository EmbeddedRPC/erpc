/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>

#include "erpc_server_setup.h"
#include "c_erpc_matrix_multiply_server.h"
#include "erpc_matrix_multiply_common.h"
#include "erpc_error_handler.h"
#include "erpc_uart_zephyr_transport.hpp"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define UART_DEVICE_NODE DT_CHOSEN(zephyr_console)

/*******************************************************************************
 * Variables
 ******************************************************************************/
static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);
erpc_server_t server;

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

int main(void)
{
    if (!device_is_ready(uart_dev))
    {
        printk("UART device not found!");
        return -1;
    }

    /* UART transport layer initialization */
    erpc_transport_t transport;

    transport = erpc_transport_zephyr_uart_init((void *)uart_dev);

    /* MessageBufferFactory initialization */
    erpc_mbf_t message_buffer_factory;
    message_buffer_factory = erpc_mbf_dynamic_init();

    /* eRPC server side initialization */
    server = erpc_server_init(transport, message_buffer_factory);

    /* adding the service to the server */
    erpc_service_t service = create_MatrixMultiplyService_service();
    erpc_add_service_to_server(server, service);

    for (;;)
    {
        /* process message */
        erpc_status_t status = erpc_server_poll(server);

        /* handle error status */
        if (status != (erpc_status_t)kErpcStatus_Success)
        {
            /* print error description */
            erpc_error_handler(status, 0);

            /* removing the service from the server */
            erpc_remove_service_from_server(server, service);
            destroy_MatrixMultiplyService_service(service);

            /* stop erpc server */
            erpc_server_stop(server);

            /* print error description */
            erpc_server_deinit(server);

            /* exit program loop */
            break;
        }

        /* do other tasks */
        int32_t i;
        for (i = 0; i < 10000; i++)
        {
        }
    }

    for (;;)
    {
    }
}
