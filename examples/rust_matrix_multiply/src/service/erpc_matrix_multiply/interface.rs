/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

pub trait MatrixMultiplyService {
    fn matrix_multiply(&mut self, matrix1: &[[u32; 5]; 5], matrix2: &[[u32; 5]; 5], result_matrix: &mut [[u32; 5]; 5]);
}

pub const MATRIX_MULTIPLY_SERVICE_ID: u8 = 1;

pub const MATRIX_MULTIPLY_SERVICE_MATRIX_MULTIPLY_ID: u8 = 1;

