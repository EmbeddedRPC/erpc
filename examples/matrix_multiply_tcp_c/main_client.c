/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_client_setup.h"
#include "c_erpc_matrix_multiply_client.h"
#include "erpc_matrix_multiply_common.h"

#include "erpc_client_setup.h"
#include "erpc_transport_setup.h"
#include "erpc_mbf_setup.h"

#include "stdio.h"
#include "stdlib.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define MATRIX_ITEM_MAX_VALUE     (50)


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
            printf("%4i ", matrix_ptr[a][b]);
        }
        printf("\r\n");
    }
}

int main()
{
    Matrix matrix1 = {0}, matrix2 = {0}, result_matrix = {0};

    erpc_transport_t transport;
    erpc_mbf_t message_buffer_factory;
    erpc_client_t client_manager;

    /* Init eRPC client infrastructure */
    transport = erpc_transport_tcp_init(EXAMPLE_TCP_HOST, EXAMPLE_TCP_PORT, false);
    message_buffer_factory = erpc_mbf_dynamic_init();
    client_manager = erpc_client_init(transport, message_buffer_factory);

    /* Init eRPC client TextService service */
    initMatrixMultiplyService_client(client_manager);

    char option;
    do 
    {
        /* Do eRPC call */
        fill_matrices(matrix1, matrix2);

        printf("\r\nMatrix #1");
        printf("\r\n=========\r\n");
        print_matrix(matrix1);

        printf("\r\nMatrix #2");
        printf("\r\n=========\r\n");
        print_matrix(matrix2);

        printf("\r\neRPC request is sent to the server\r\n");

        erpcMatrixMultiply(matrix1, matrix2, result_matrix);

        printf("\r\nResult matrix");
        printf("\r\n=============\r\n");
        print_matrix(result_matrix);

        printf("Press Enter to initiate the next matrix multiplication or 'q' to quit\n");
        option = getchar ();
        while(option != '\n' && getchar() != '\n');
    } while (option != 'q');

    quitServer();

    /* Deinit objects */
    deinitMatrixMultiplyService_client();
    erpc_client_deinit(client_manager);
    erpc_mbf_dynamic_deinit(message_buffer_factory);
    erpc_transport_tcp_close(transport);
    erpc_transport_tcp_deinit(transport);
}
