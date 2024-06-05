/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use std::fmt::Display;

/// Parameter direction
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub(crate) enum ParamDirection {
    In,
    Out,
    InOut,
}

/// Display implementation for ParamDirection
impl Display for ParamDirection {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            ParamDirection::In => write!(f, "in"),
            ParamDirection::Out => write!(f, "out"),
            ParamDirection::InOut => write!(f, "inout"),
        }
    }
}