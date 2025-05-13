/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


/// Enumeration for binary operations.
#[derive(Debug, Copy, Clone)]
pub(crate) enum BinaryOperation {
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Shl,
    Shr,
    Xor,
    And,
    Or
}

/// Enumeration for unary operations.
#[derive(Debug, Copy, Clone)]
pub(crate) enum UnaryOperation {
    Nop,
    Neg,
    Inv
}