/*
 * Copyright 2016 Freescale Semiconductor, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of Mentor Graphics Corporation nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include "erpc_mbf_setup.h"
#include "erpc_server_setup.h"
#include "service/erpc_matrix_multiply/erpc_matrix_multiply_server.h"
#include "service/erpc_matrix_multiply/erpc_matrix_multiply.h"
#include "erpc_client_setup.h"
#include "erpc_transport_setup.h"

#include "erpc_transport_setup.h"
#include "assert.h"
#include "erpc_mbf_setup.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#define APP_TASK_STACK_SIZE 256


#define TASK_STACK_SIZE 300



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

    printf("Calculating the matrix multiplication...\r\n");

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

    printf("Done!\r\n");
}



int main(void)
{
    erpc_transport_t transport = erpc_transport_tcp_init("127.0.0.0", 50000, true);	


//    erpc_mbf_t message_buffer_factory = erpc_mbf_rpmsg_init(transport);
    erpc_mbf_t message_buffer_factory = erpc_mbf_static_init();



    /* eRPC server side initialization */
    erpc_server_init(transport, message_buffer_factory);


    printf("eRPC intialized\r\n");

    /* adding the service to the server */
    erpc_add_service_to_server(create_MatrixMultiplyService_service());

    printf("MatrixMultiply service added\r\n");

    /* run server */
    while(1){
            printf("Before calling server run\r\n");
	    erpc_server_run();
    	    printf("Before callign server run\r\n");
    }

//    while (1)
//    {
//        /* process message */
//        erpc_server_poll();
//    }
    return 0;
}

