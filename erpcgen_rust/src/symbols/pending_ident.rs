/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use crate::symbols::types::Type;

/// Pending identifier
pub(crate) struct PendingIdent {
    pub(crate) name: String,
    pub(crate) position: usize,
    pub(crate) check: Box<dyn Fn(&Type) -> Option<String>> // If error, return why, else return None
}

/// Implementation of PendingIdent
impl PendingIdent {
    pub(crate) fn new(name: String, position: usize, check: Box<dyn Fn(&Type) -> Option<String>>) -> Self {
        Self {
            name,
            position,
            check
        }
    }
}

/// Check if the type is a scalar
pub const CHECK_SCALAR: fn(&Type) -> Option<String> = |arg| check_scalar(arg);

/// Check if the type is a scalar
///
/// # Arguments
///
/// * `t` - The type to check
///
/// # Returns
///
/// * None if the type is a scalar, else a string with the error
fn check_scalar(t: &Type) -> Option<String> {
    if t.is_scalar() {
        None
    } else {
        Some(format!("Type {:?} is not a scalar type", t))
    }
}

/// Pending error
pub(crate) struct PendingError {
    pub(crate) name: String,
    pub(crate) position: usize,
    pub(crate) error: String
}

/// Implementation of PendingError
impl PendingError {
    pub(crate) fn new(name: String, position: usize, error: String) -> Self {
        Self {
            name,
            position,
            error
        }
    }
}

