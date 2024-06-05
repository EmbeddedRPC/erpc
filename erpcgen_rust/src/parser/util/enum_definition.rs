/*
 * Copyright 2024 Marek Mišík(nxf76107)
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use crate::symbols::enum_member::EnumMember;
use crate::symbols::value::Value;

/// Resolves the value of the enum member
///
/// # Arguments
///
/// * `preceding_enum_member` - The preceding enum member
///
/// # Returns
///
/// The value of the enum member
pub(crate) fn resolve_enum_member_value(preceding_enum_member: Option<&EnumMember>) -> Value {
    match preceding_enum_member {
        Some(member) => {
            Value::Int64(member.value.to_i64() + 1)
        }
        None => {
            Value::Int64(0)
        }
    }
}
