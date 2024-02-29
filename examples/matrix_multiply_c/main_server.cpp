/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_matrix_multiply_server.hpp"
#include "config.h"

#include "erpc_basic_codec.hpp"
#include "erpc_crc16.hpp"
#include "erpc_message_buffer.hpp"
#include "erpc_simple_server.hpp"
#include "erpc_tcp_transport.hpp"

#include "erpc_error_handler.h"

#include <iostream>

using namespace erpcShim;
using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

class MyMessageBufferFactory : public MessageBufferFactory
{
public:
    virtual MessageBuffer create()
    {
        uint8_t *buf = new uint8_t[ERPC_DEFAULT_BUFFER_SIZE];
        return MessageBuffer(buf, ERPC_DEFAULT_BUFFER_SIZE);
    }

    virtual void dispose(MessageBuffer *buf)
    {
        erpc_assert(buf);
        if (*buf)
        {
            delete[] buf->get();
        }
    }
};

class MatrixMultiplyService : public MatrixMultiplyService_interface
{
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
};

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

int main()
{
    erpc_status_t status;

    /* Init eRPC server components */
    TCPTransport transport(ERPC_HOSTNAME, ERPC_PORT, true);
    MyMessageBufferFactory msgFactory;
    BasicCodecFactory codecFactory;
    SimpleServer server;
    Crc16 crc16;

    /* Init service implementation and handler */
    MatrixMultiplyService textServiceImpl;
    MatrixMultiplyService_service textService(&textServiceImpl);

    /* Setup transport */
    transport.setCrc16(&crc16);
    status = transport.open();

    if (status != kErpcStatus_Success)
    {
        /* Print error description */
        erpc_error_handler(status, 0);
        return -1;
    }

    /* Setup server */
    server.setTransport(&transport);
    server.setCodecFactory(&codecFactory);
    server.setMessageBufferFactory(&msgFactory);

    /* add custom service implementation to the server */
    server.addService(&textService);

    /* poll for requests */
    status = server.poll();

    /* handle error status */
    if (status != kErpcStatus_Success)
    {
        /* print error description */
        erpc_error_handler(status, 0);
    }

    /* Close transport */
    transport.close();
}
