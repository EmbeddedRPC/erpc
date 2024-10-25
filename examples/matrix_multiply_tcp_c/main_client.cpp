/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_matrix_multiply_client.hpp"
#include "erpc_matrix_multiply_interface.hpp"

#include "erpc_basic_codec.hpp"
#include "erpc_client_manager.h"
#include "erpc_tcp_transport.hpp"

#include "erpc_error_handler.h"

#include <iostream>
#include <iomanip>
#include <cstdlib>


using namespace erpcShim;
using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////
#define MATRIX_ITEM_MAX_VALUE     (50)

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

class MyMessageBufferFactory : public MessageBufferFactory
{
public:
    virtual MessageBuffer create()
    {
        uint8_t *buf = new uint8_t[1024];
        return MessageBuffer(buf, 1024);
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
            std::cout << std::setw(4) << matrix_ptr[a][b] << " ";
        }
        std::cout << std::endl;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

int main()
{
    Matrix matrix1 = {0}, matrix2 = {0}, result_matrix = {0};

    erpc_status_t status;

    /* Init eRPC client components */
    TCPTransport transport(EXAMPLE_TCP_HOST, EXAMPLE_TCP_PORT, false);
    MyMessageBufferFactory msgFactory;
    BasicCodecFactory basicCodecFactory;
    Crc16 crc16;
    ClientManager clientManager;

    /* Init transport */
    transport.setCrc16(&crc16);
    status = transport.open();

    if (status != kErpcStatus_Success)
    {
        /* print error description */
        erpc_error_handler(status, 0);
        return -1;
    }

    /* Init client manager */
    clientManager.setMessageBufferFactory(&msgFactory);
    clientManager.setTransport(&transport);
    clientManager.setCodecFactory(&basicCodecFactory);

    /* Init eRPC client MatrixMultiply service */
    MatrixMultiplyService_client client(&clientManager);

    std::string choice;
    while (true)
    {
        fill_matrices(matrix1, matrix2);

        std::cout << "Matrix #1" << std::endl << "=========" << std::endl;
        print_matrix(matrix1);

        std::cout << std::endl << "Matrix #2" << std::endl << "=========" << std::endl;
        print_matrix(matrix2);

 
        /* Do eRPC call */
        std::cout << "eRPC request is sent to the server" << std::endl;
        client.erpcMatrixMultiply(matrix1, matrix2, result_matrix);


        std::cout << std::endl << "Result matrix" << std::endl << "=========" << std::endl;
        print_matrix(result_matrix);

        std::cout << "Press Enter to initiate the next matrix multiplication or 'q' to quit: ";
        std::getline(std::cin, choice);

        if (choice == "q") {
            break;
        }
    }
    
    client.quitServer();

    /* Deinit objects */
    transport.close();
}
