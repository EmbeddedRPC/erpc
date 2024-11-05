/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Generated by erpcgen 1.13.0 on Thu Apr 18 09:59:39 2024.
 *
 * AUTOGENERATED - DO NOT EDIT
 */


#if !defined(_erpc_matrix_multiply_client_hpp_)
#define _erpc_matrix_multiply_client_hpp_

#include "erpc_matrix_multiply_interface.hpp"

#include "erpc_client_manager.h"

namespace erpcShim
{

class MatrixMultiplyService_client: public MatrixMultiplyService_interface
{
    public:
        MatrixMultiplyService_client(erpc::ClientManager *manager);

        virtual ~MatrixMultiplyService_client();

        virtual void erpcMatrixMultiply(Matrix matrix1, Matrix matrix2, Matrix result_matrix);

    protected:
        erpc::ClientManager *m_clientManager;
};

} // erpcShim


#endif // _erpc_matrix_multiply_client_hpp_
