/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "c_erpc_matrix_multiply_server.h"

#include "erpc_server_setup.h"
#include "erpc_transport_setup.h"
#include "erpc_mbf_setup.h"

#include "erpc_error_handler.h"

#include <stdio.h>

/*******************************************************************************
 * Variables
 ******************************************************************************/
bool g_runServer = true;

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
 * @brief Stop the server
 */
void quitServer()
{
    g_runServer = false;
}

/*!
 * @brief Main function
 */
int main()
{
    erpc_transport_t transport;
    erpc_mbf_t message_buffer_factory;
    erpc_server_t server;
    erpc_service_t service = create_MatrixMultiplyService_service();

    /* Init eRPC server infrastructure */
    transport = erpc_transport_tcp_init(EXAMPLE_TCP_HOST, EXAMPLE_TCP_PORT, true);
    message_buffer_factory = erpc_mbf_dynamic_init();
    server = erpc_server_init(transport, message_buffer_factory);

    /* Add custom service implementation to the server */
    erpc_add_service_to_server(server, service);

    while (g_runServer)
    {
        /* Poll for request */
        erpc_status_t err = erpc_server_poll(server);

        /* Handle error status */
        if (err != kErpcStatus_Success)
        {
            /* Print error description */
            erpc_error_handler(err, 0);
        }
    }

    /* Deinit objects */
    destroy_MatrixMultiplyService_service(service);
    erpc_server_deinit(server);
    erpc_mbf_dynamic_deinit(message_buffer_factory);
    erpc_transport_tcp_close(transport);
    erpc_transport_tcp_deinit(transport);
}
