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
#include "service/erpc_matrix_multiply/erpc_matrix_multiply_server.h"
#include "service/erpc_matrix_multiply/erpc_matrix_multiply.h"

#include "erpc_transport_setup.h"
#include "assert.h"
#include "erpc_mbf_setup.h"
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#define APP_TASK_STACK_SIZE 256


#define TASK_STACK_SIZE 300


#include "erpc_client_setup.h"
#include "erpc_transport_setup.h"

int main()
{
    char invoer;
    /* Matrices definitions */
    Matrix matrix1, matrix2, result_matrix = {{1}};

    /* init eRPC client environment */
    /* TCP transport layer initialization */
    erpc_transport_t transport = erpc_transport_tcp_init("0.0.0.0", 50000, false);	

    /* MessageBufferFactory initialization */
    erpc_mbf_t message_buffer_factory = erpc_mbf_static_init();
 
    /* eRPC client side initialization */
    erpc_client_init(transport, message_buffer_factory);

   
    /* other code like print result matrix */
    printf("Initializing client side\n");


    /* other code like init matrix1 and matrix2 values */
    
    int i,j;
   
    for (i=0;i<matrix_size;i++)
	for (j=0;j<matrix_size;j++)
		matrix1[i][j] = (rand()%100);

    for (i=0;i<matrix_size;i++)
	for(j=0;j<matrix_size;j++)
		matrix2[i][j] = (rand()%100);

    /* other code like print result matrix */
    printf("Calling eRPC matrix multiply on server side\n");

    while(1){	
    /* call eRPC functions */
    erpcMatrixMultiply(matrix1, matrix2, result_matrix);

    /* other code like print result matrix */
    printf("Here is the result of the matrix multiplication: \n");


    for (i=0;i<matrix_size;i++){
        for(j=0;j<matrix_size;j++)
                printf("result_matrix[%d][%d] = %d\t",i,j,result_matrix[i][j]);
	printf("\n");    
    }
//    }
    getchar();
//    scanf("%c",&invoer);
    continue;	
}
    /* eRPC client side initialization */
    erpc_client_deinit();


    return 0;
}



