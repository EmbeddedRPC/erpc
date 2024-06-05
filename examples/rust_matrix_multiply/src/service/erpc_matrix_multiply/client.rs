/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use crate::service::erpc_matrix_multiply::interface::{
    MatrixMultiplyService, MATRIX_MULTIPLY_SERVICE_ID, MATRIX_MULTIPLY_SERVICE_MATRIX_MULTIPLY_ID,
};
use erpc_rust::client::ClientManager;
use erpc_rust::codec::{Codec, MsgInfo};
use erpc_rust::message_type::MessageType;
use erpc_rust::transport::Transport;

pub struct MatrixMultiplyClient<T: Codec + Clone, S: Transport> {
    client_manager: ClientManager<T, S>,
}

impl<T: Codec + Clone, S: Transport> MatrixMultiplyService for MatrixMultiplyClient<T, S> {
    fn matrix_multiply(
        &mut self,
        matrix1: &[[u32; 5]; 5],
        matrix2: &[[u32; 5]; 5],
        result_matrix: &mut [[u32; 5]; 5],
    ) {
        let mut request = self.client_manager.create_request(false);
        let sequence = request.sequence();

        request.codec.start_write_message(MsgInfo {
            msg_type: MessageType::Invocation,
            service: MATRIX_MULTIPLY_SERVICE_ID,
            request: MATRIX_MULTIPLY_SERVICE_MATRIX_MULTIPLY_ID,
            sequence,
        });

        for _i0 in matrix1 {
            for _i1 in _i0 {
                request.codec.write_u32(*_i1);
            }
        }
        for _i0 in matrix2 {
            for _i1 in _i0 {
                request.codec.write_u32(*_i1);
            }
        }

        self.client_manager.perform_request(&mut request).unwrap();

        for _i0 in 0..5 {
            for _i1 in 0..5 {
                result_matrix[_i0][_i1] = request.codec.read_u32();
            }
        }
    }
}

impl<T: Codec + Clone, S: Transport> MatrixMultiplyClient<T, S> {
    pub fn new(client_manager: ClientManager<T, S>) -> Self {
        Self { client_manager }
    }
}
