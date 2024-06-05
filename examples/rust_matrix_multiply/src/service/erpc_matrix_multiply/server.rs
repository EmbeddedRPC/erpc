/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use crate::service::erpc_matrix_multiply::interface::{
    MatrixMultiplyService, MATRIX_MULTIPLY_SERVICE_ID, MATRIX_MULTIPLY_SERVICE_MATRIX_MULTIPLY_ID,
};
use erpc_rust::codec::{Codec, MsgInfo};
use erpc_rust::message_type::MessageType;
use erpc_rust::server::Service;
use std::collections::HashMap;

type HandlerFunction<T, S> = fn(&mut T, &mut S, u32) -> Result<(), String>;

pub struct MatrixMultiplyServiceService<T: MatrixMultiplyService, S: Codec> {
    handler: T,
    handlers: HashMap<u8, HandlerFunction<T, S>>,
}

impl<T: MatrixMultiplyService, S: Codec> MatrixMultiplyServiceService<T, S> {
    pub fn new(handler: T) -> Self {
        let mut handlers = HashMap::new();
        handlers.insert(
            MATRIX_MULTIPLY_SERVICE_MATRIX_MULTIPLY_ID,
            MatrixMultiplyServiceService::<T, S>::matrix_multiply_handler as HandlerFunction<T, S>,
        );
        Self { handler, handlers }
    }
}

impl<T: MatrixMultiplyService, S: Codec> Service<S> for MatrixMultiplyServiceService<T, S> {
    fn service_id(&self) -> u8 {
        MATRIX_MULTIPLY_SERVICE_ID
    }

    fn handle_invocation(&mut self, method_id: u8, sequence: u32, codec: &mut S) {
        let handler = self.handlers.get(&method_id).unwrap();
        handler(&mut self.handler, codec, sequence).unwrap();
    }
}

impl<T: MatrixMultiplyService, S: Codec> MatrixMultiplyServiceService<T, S> {
    fn matrix_multiply_handler(
        handler: &mut T,
        codec: &mut S,
        sequence: u32,
    ) -> Result<(), String> {
        let mut matrix1 = [[u32::default(); 5]; 5];
        let mut matrix2 = [[u32::default(); 5]; 5];
        let mut result_matrix = [[u32::default(); 5]; 5];

        for i0 in 0..5 {
            for i1 in 0..5 {
                matrix1[i0][i1] = codec.read_u32();
            }
        }
        for i0 in 0..5 {
            for i1 in 0..5 {
                matrix2[i0][i1] = codec.read_u32();
            }
        }

        handler.matrix_multiply(&matrix1, &matrix2, &mut result_matrix);

        codec.reset();

        codec.start_write_message(MsgInfo {
            msg_type: MessageType::Reply,
            service: MATRIX_MULTIPLY_SERVICE_ID,
            request: MATRIX_MULTIPLY_SERVICE_MATRIX_MULTIPLY_ID,
            sequence,
        });

        for i0 in 0..5 {
            for i1 in 0..5 {
                codec.write_u32(result_matrix[i0][i1]);
            }
        }

        Ok(())
    }
}
