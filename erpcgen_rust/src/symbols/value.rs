/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use std::collections::HashMap;
use std::hash::{Hash, Hasher};
use std::ops::{Add, BitAnd, BitOr, BitXor, Div, Mul, Neg, Not, Rem, Shl, Shr, Sub};
use crate::symbols::types::Type;

/// Value
#[derive(Debug, Clone, PartialEq)]
pub(crate) enum Value {
    Bool(bool),
    Int8(i8),
    Int16(i16),
    Int32(i32),
    Int64(i64),
    Uint8(u8),
    Uint16(u16),
    Uint32(u32),
    UInt64(u64),
    Float(f32),
    Double(f64),
    String(String),
    Struct {
        fields: HashMap<String, Value>,
    },
    Enum {
        value: i32,
    },
    Union {
        fields: HashMap<String, Value>,
    },
    Array {
        elements: Vec<Value>,
        dimension_sizes: Vec<usize>,
    },
    List {
        elements: Vec<Value>,
    },
    ReferencedValue {
        name: String,
        prefix: Option<String>,
        value: Box<Value>,
    },
    RuntimeValue { // The type of the value needs to be checked during parsing
        name: String,
    }
}

/// Hash implementation for Value
impl Hash for Value {
    fn hash<H: Hasher>(&self, state: &mut H) {
        match self { 
            Value::Bool(v) => v.hash(state),
            Value::Int8(v) => v.hash(state),
            Value::Int16(v) => v.hash(state),
            Value::Int32(v) => v.hash(state),
            Value::Int64(v) => v.hash(state),
            Value::Uint8(v) => v.hash(state),
            Value::Uint16(v) => v.hash(state),
            Value::Uint32(v) => v.hash(state),
            Value::UInt64(v) => v.hash(state),
            Value::Float(v) => v.to_bits().hash(state),
            Value::Double(v) => v.to_bits().hash(state),
            Value::String(v) => v.hash(state),
            Value::Struct { fields } => {
                for (name, value) in fields {
                    name.hash(state);
                    value.hash(state);
                }
            }
            Value::Enum { value } => value.hash(state),
            Value::Union { fields } => {
                for (name, value) in fields {
                    name.hash(state);
                    value.hash(state);
                }
            }
            Value::Array { elements, dimension_sizes } => {
                for value in elements {
                    value.hash(state);
                }
                for size in dimension_sizes {
                    size.hash(state);
                }
            }
            Value::List { elements } => {
                for value in elements {
                    value.hash(state);
                }
            }
            Value::ReferencedValue { name, value, .. } => {
                name.hash(state);
                value.hash(state);
            }
            Value::RuntimeValue { name } => {
                name.hash(state);
            }
        }
    }
}

/// Mathematical addition implementation for Value
impl Add for Value {
    type Output = Self;

    /// Add two values
    ///
    /// # Arguments
    ///
    /// * `rhs` - The right hand side value
    ///
    /// # Returns
    ///
    /// * The result of the addition
    fn add(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Value::Int8(lhs), Value::Int8(rhs)) => Value::Int8(lhs + rhs),
            (Value::Int16(lhs), Value::Int16(rhs)) => Value::Int16(lhs + rhs),
            (Value::Int32(lhs), Value::Int32(rhs)) => Value::Int32(lhs + rhs),
            (Value::Int64(lhs), Value::Int64(rhs)) => Value::Int64(lhs + rhs),
            (Value::Uint8(lhs), Value::Uint8(rhs)) => Value::Uint8(lhs + rhs),
            (Value::Uint16(lhs), Value::Uint16(rhs)) => Value::Uint16(lhs + rhs),
            (Value::Uint32(lhs), Value::Uint32(rhs)) => Value::Uint32(lhs + rhs),
            (Value::UInt64(lhs), Value::UInt64(rhs)) => Value::UInt64(lhs + rhs),
            (Value::Float(lhs), Value::Float(rhs)) => Value::Float(lhs + rhs),
            (Value::Double(lhs), Value::Double(rhs)) => Value::Double(lhs + rhs),
            (Value::UInt64(lhs), Value::Int64(rhs)) => Value::Int64(lhs as i64 + rhs),
            (Value::Int64(lhs), Value::UInt64(rhs)) => Value::Int64(lhs + rhs as i64),
            (Value::Int64(lhs), Value::Double(rhs)) => Value::Double(lhs as f64 + rhs),
            (Value::Double(lhs), Value::Int64(rhs)) => Value::Double(lhs + rhs as f64),
            (Value::UInt64(lhs), Value::Double(rhs)) => Value::Double(lhs as f64 + rhs),
            (Value::Double(lhs), Value::UInt64(rhs)) => Value::Double(lhs + rhs as f64),
            (lhs, rhs) => {
                panic!(
                    "Bitwise XOR not supported for types: {:?} and {:?}",
                    lhs, rhs
                );
            }
        }
    }
}

/// Mathematical subtraction implementation for Value
impl Sub for Value {
    type Output = Self;

    /// Subtract two values
    ///
    /// # Arguments
    ///
    /// * `rhs` - The right hand side value
    ///
    /// # Returns
    ///
    /// * The result of the subtraction
    fn sub(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Value::Int8(lhs), Value::Int8(rhs)) => Value::Int8(lhs - rhs),
            (Value::Int16(lhs), Value::Int16(rhs)) => Value::Int16(lhs - rhs),
            (Value::Int32(lhs), Value::Int32(rhs)) => Value::Int32(lhs - rhs),
            (Value::Int64(lhs), Value::Int64(rhs)) => Value::Int64(lhs - rhs),
            (Value::Uint8(lhs), Value::Uint8(rhs)) => Value::Uint8(lhs - rhs),
            (Value::Uint16(lhs), Value::Uint16(rhs)) => Value::Uint16(lhs - rhs),
            (Value::Uint32(lhs), Value::Uint32(rhs)) => Value::Uint32(lhs - rhs),
            (Value::UInt64(lhs), Value::UInt64(rhs)) => Value::UInt64(lhs - rhs),
            (Value::Float(lhs), Value::Float(rhs)) => Value::Float(lhs - rhs),
            (Value::Double(lhs), Value::Double(rhs)) => Value::Double(lhs - rhs),
            (Value::UInt64(lhs), Value::Int64(rhs)) => Value::Int64(lhs as i64 - rhs),
            (Value::Int64(lhs), Value::UInt64(rhs)) => Value::Int64(lhs - rhs as i64),
            (Value::Int64(lhs), Value::Double(rhs)) => Value::Double(lhs as f64 - rhs),
            (Value::Double(lhs), Value::Int64(rhs)) => Value::Double(lhs - rhs as f64),
            (Value::UInt64(lhs), Value::Double(rhs)) => Value::Double(lhs as f64 - rhs),
            (Value::Double(lhs), Value::UInt64(rhs)) => Value::Double(lhs - rhs as f64),
            (lhs, rhs) => {
                panic!(
                    "Bitwise XOR not supported for types: {:?} and {:?}",
                    lhs, rhs
                );
            }
        }
    }
}

/// Mathematical multiplication implementation for Value
impl Mul for Value {
    type Output = Self;

    /// Multiply two values
    ///
    /// # Arguments
    ///
    /// * `rhs` - The right hand side value
    ///
    /// # Returns
    ///
    /// * The result of the multiplication
    fn mul(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Value::Int8(lhs), Value::Int8(rhs)) => Value::Int8(lhs * rhs),
            (Value::Int16(lhs), Value::Int16(rhs)) => Value::Int16(lhs * rhs),
            (Value::Int32(lhs), Value::Int32(rhs)) => Value::Int32(lhs * rhs),
            (Value::Int64(lhs), Value::Int64(rhs)) => Value::Int64(lhs * rhs),
            (Value::Uint8(lhs), Value::Uint8(rhs)) => Value::Uint8(lhs * rhs),
            (Value::Uint16(lhs), Value::Uint16(rhs)) => Value::Uint16(lhs * rhs),
            (Value::Uint32(lhs), Value::Uint32(rhs)) => Value::Uint32(lhs * rhs),
            (Value::UInt64(lhs), Value::UInt64(rhs)) => Value::UInt64(lhs * rhs),
            (Value::Float(lhs), Value::Float(rhs)) => Value::Float(lhs * rhs),
            (Value::Double(lhs), Value::Double(rhs)) => Value::Double(lhs * rhs),
            (Value::UInt64(lhs), Value::Int64(rhs)) => Value::Int64(lhs as i64 * rhs),
            (Value::Int64(lhs), Value::UInt64(rhs)) => Value::Int64(lhs * rhs as i64),
            (Value::Int64(lhs), Value::Double(rhs)) => Value::Double(lhs as f64 * rhs),
            (Value::Double(lhs), Value::Int64(rhs)) => Value::Double(lhs * rhs as f64),
            (Value::UInt64(lhs), Value::Double(rhs)) => Value::Double(lhs as f64 * rhs),
            (Value::Double(lhs), Value::UInt64(rhs)) => Value::Double(lhs * rhs as f64),
            (lhs, rhs) => {
                panic!(
                    "Bitwise XOR not supported for types: {:?} and {:?}",
                    lhs, rhs
                );
            }
        }
    }
}

/// Mathematical division implementation for Value
impl Div for Value {
    type Output = Self;

    /// Divide two values
    ///
    /// # Arguments
    ///
    /// * `rhs` - The right hand side value
    ///
    /// # Returns
    ///
    /// * The result of the division
    fn div(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Value::Int8(lhs), Value::Int8(rhs)) => Value::Int8(lhs / rhs),
            (Value::Int16(lhs), Value::Int16(rhs)) => Value::Int16(lhs / rhs),
            (Value::Int32(lhs), Value::Int32(rhs)) => Value::Int32(lhs / rhs),
            (Value::Int64(lhs), Value::Int64(rhs)) => Value::Int64(lhs / rhs),
            (Value::Uint8(lhs), Value::Uint8(rhs)) => Value::Uint8(lhs / rhs),
            (Value::Uint16(lhs), Value::Uint16(rhs)) => Value::Uint16(lhs / rhs),
            (Value::Uint32(lhs), Value::Uint32(rhs)) => Value::Uint32(lhs / rhs),
            (Value::UInt64(lhs), Value::UInt64(rhs)) => Value::UInt64(lhs / rhs),
            (Value::Float(lhs), Value::Float(rhs)) => Value::Float(lhs / rhs),
            (Value::Double(lhs), Value::Double(rhs)) => Value::Double(lhs / rhs),
            (Value::UInt64(lhs), Value::Int64(rhs)) => Value::Int64(lhs as i64 / rhs),
            (Value::Int64(lhs), Value::UInt64(rhs)) => Value::Int64(lhs / rhs as i64),
            (Value::Int64(lhs), Value::Double(rhs)) => Value::Double(lhs as f64 / rhs),
            (Value::Double(lhs), Value::Int64(rhs)) => Value::Double(lhs / rhs as f64),
            (Value::UInt64(lhs), Value::Double(rhs)) => Value::Double(lhs as f64 / rhs),
            (Value::Double(lhs), Value::UInt64(rhs)) => Value::Double(lhs / rhs as f64),
            (lhs, rhs) => {
                panic!(
                    "Bitwise XOR not supported for types: {:?} and {:?}",
                    lhs, rhs
                );
            }
        }
    }
}

/// Mathematical remainder implementation for Value
impl Rem for Value {
    type Output = Self;

    /// Calculate the remainder of two values
    ///
    /// # Arguments
    ///
    /// * `rhs` - The right hand side value
    ///
    /// # Returns
    ///
    /// * The result of the remainder
    fn rem(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Value::Int8(lhs), Value::Int8(rhs)) => Value::Int8(lhs % rhs),
            (Value::Int16(lhs), Value::Int16(rhs)) => Value::Int16(lhs % rhs),
            (Value::Int32(lhs), Value::Int32(rhs)) => Value::Int32(lhs % rhs),
            (Value::Int64(lhs), Value::Int64(rhs)) => Value::Int64(lhs % rhs),
            (Value::Uint8(lhs), Value::Uint8(rhs)) => Value::Uint8(lhs % rhs),
            (Value::Uint16(lhs), Value::Uint16(rhs)) => Value::Uint16(lhs % rhs),
            (Value::Uint32(lhs), Value::Uint32(rhs)) => Value::Uint32(lhs % rhs),
            (Value::UInt64(lhs), Value::UInt64(rhs)) => Value::UInt64(lhs % rhs),
            (Value::Float(lhs), Value::Float(rhs)) => Value::Float(lhs % rhs),
            (Value::Double(lhs), Value::Double(rhs)) => Value::Double(lhs % rhs),
            (Value::UInt64(lhs), Value::Int64(rhs)) => Value::Int64(lhs as i64 % rhs),
            (Value::Int64(lhs), Value::UInt64(rhs)) => Value::Int64(lhs % rhs as i64),
            (Value::Int64(lhs), Value::Double(rhs)) => Value::Double(lhs as f64 % rhs),
            (Value::Double(lhs), Value::Int64(rhs)) => Value::Double(lhs % rhs as f64),
            (Value::UInt64(lhs), Value::Double(rhs)) => Value::Double(lhs as f64 % rhs),
            (Value::Double(lhs), Value::UInt64(rhs)) => Value::Double(lhs % rhs as f64),
            (lhs, rhs) => {
                panic!(
                    "Bitwise XOR not supported for types: {:?} and {:?}",
                    lhs, rhs
                );
            }
        }
    }
}

/// Bitwise OR implementation for Value
impl BitOr for Value {
    type Output = Self;

    /// Perform a bitwise OR operation on two values
    ///
    /// # Arguments
    ///
    /// * `rhs` - The right hand side value
    ///
    /// # Returns
    ///
    /// * The result of the bitwise OR operation
    fn bitor(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Value::Int8(lhs), Value::Int8(rhs)) => Value::Int8(lhs | rhs),
            (Value::Int16(lhs), Value::Int16(rhs)) => Value::Int16(lhs | rhs),
            (Value::Int32(lhs), Value::Int32(rhs)) => Value::Int32(lhs | rhs),
            (Value::Int64(lhs), Value::Int64(rhs)) => Value::Int64(lhs | rhs),
            (Value::Uint8(lhs), Value::Uint8(rhs)) => Value::Uint8(lhs | rhs),
            (Value::Uint16(lhs), Value::Uint16(rhs)) => Value::Uint16(lhs | rhs),
            (Value::Uint32(lhs), Value::Uint32(rhs)) => Value::Uint32(lhs | rhs),
            (Value::UInt64(lhs), Value::UInt64(rhs)) => Value::UInt64(lhs | rhs),
            (Value::UInt64(lhs), Value::Int64(rhs)) => Value::Int64(lhs as i64 | rhs),
            (Value::Int64(lhs), Value::UInt64(rhs)) => Value::Int64(lhs | rhs as i64),
            (Value::Int64(lhs), Value::Double(rhs)) => Value::Int64(lhs | rhs as i64),
            (Value::Double(lhs), Value::Int64(rhs)) => Value::Int64(lhs as i64 | rhs),
            (Value::UInt64(lhs), Value::Double(rhs)) => Value::UInt64(lhs | rhs as u64),
            (Value::Double(lhs), Value::UInt64(rhs)) => Value::UInt64(lhs as u64 | rhs),
            (lhs, rhs) => {
                panic!(
                    "Bitwise XOR not supported for types: {:?} and {:?}",
                    lhs, rhs
                );
            }
        }
    }
}

/// Bitwise AND implementation for Value
impl BitAnd for Value {
    type Output = Self;

    /// Perform a bitwise AND operation on two values
    ///
    /// # Arguments
    ///
    /// * `rhs` - The right hand side value
    ///
    /// # Returns
    ///
    /// * The result of the bitwise AND operation
    fn bitand(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Value::Int8(lhs), Value::Int8(rhs)) => Value::Int8(lhs & rhs),
            (Value::Int16(lhs), Value::Int16(rhs)) => Value::Int16(lhs & rhs),
            (Value::Int32(lhs), Value::Int32(rhs)) => Value::Int32(lhs & rhs),
            (Value::Int64(lhs), Value::Int64(rhs)) => Value::Int64(lhs & rhs),
            (Value::Uint8(lhs), Value::Uint8(rhs)) => Value::Uint8(lhs & rhs),
            (Value::Uint16(lhs), Value::Uint16(rhs)) => Value::Uint16(lhs & rhs),
            (Value::Uint32(lhs), Value::Uint32(rhs)) => Value::Uint32(lhs & rhs),
            (Value::UInt64(lhs), Value::UInt64(rhs)) => Value::UInt64(lhs & rhs),
            (Value::UInt64(lhs), Value::Int64(rhs)) => Value::UInt64(lhs & rhs as u64),
            (Value::Int64(lhs), Value::UInt64(rhs)) => Value::UInt64(lhs as u64 & rhs),
            (Value::Int64(lhs), Value::Double(rhs)) => Value::Int64(lhs as i64 & rhs as i64),
            (Value::Double(lhs), Value::Int64(rhs)) => Value::Int64(lhs as i64 & rhs as i64),
            (Value::UInt64(lhs), Value::Double(rhs)) => Value::UInt64(lhs & rhs as u64),
            (Value::Double(lhs), Value::UInt64(rhs)) => Value::UInt64(lhs as u64 & rhs),
            (lhs, rhs) => {
                panic!(
                    "Bitwise XOR not supported for types: {:?} and {:?}",
                    lhs, rhs
                );
            }
        }
    }
}

/// Bitwise XOR implementation for Value
impl BitXor for Value {
    type Output = Self;

    /// Perform a bitwise XOR operation on two values
    ///
    /// # Arguments
    ///
    /// * `rhs` - The right hand side value
    ///
    /// # Returns
    ///
    /// * The result of the bitwise XOR operation
    fn bitxor(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Value::Int8(lhs), Value::Int8(rhs)) => Value::Int8(lhs ^ rhs),
            (Value::Int16(lhs), Value::Int16(rhs)) => Value::Int16(lhs ^ rhs),
            (Value::Int32(lhs), Value::Int32(rhs)) => Value::Int32(lhs ^ rhs),
            (Value::Int64(lhs), Value::Int64(rhs)) => Value::Int64(lhs ^ rhs),
            (Value::Uint8(lhs), Value::Uint8(rhs)) => Value::Uint8(lhs ^ rhs),
            (Value::Uint16(lhs), Value::Uint16(rhs)) => Value::Uint16(lhs ^ rhs),
            (Value::Uint32(lhs), Value::Uint32(rhs)) => Value::Uint32(lhs ^ rhs),
            (Value::UInt64(lhs), Value::UInt64(rhs)) => Value::UInt64(lhs ^ rhs),
            (Value::UInt64(lhs), Value::Int64(rhs)) => Value::UInt64(lhs ^ rhs as u64),
            (Value::Int64(lhs), Value::UInt64(rhs)) => Value::UInt64(lhs as u64 ^ rhs),
            (Value::Int64(lhs), Value::Double(rhs)) => Value::Int64(lhs ^ rhs as i64),
            (Value::Double(lhs), Value::Int64(rhs)) => Value::Int64(lhs as i64 ^ rhs),
            (Value::UInt64(lhs), Value::Double(rhs)) => Value::UInt64(lhs ^ rhs as u64),
            (Value::Double(lhs), Value::UInt64(rhs)) => Value::UInt64(lhs as u64 ^ rhs),
            (lhs, rhs) => {
                panic!(
                    "Bitwise XOR not supported for types: {:?} and {:?}",
                    lhs, rhs
                );
            }
        }
    }
}

/// Bitwise shift left implementation for Value
impl Shl for Value {
    type Output = Self;

    /// Perform a bitwise shift left operation on two values
    ///
    /// # Arguments
    ///
    /// * `rhs` - The right hand side value
    ///
    /// # Returns
    ///
    /// * The result of the bitwise shift left operation
    fn shl(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Value::Int8(lhs), Value::Int8(rhs)) => Value::Int8(lhs << rhs),
            (Value::Int16(lhs), Value::Int16(rhs)) => Value::Int16(lhs << rhs),
            (Value::Int32(lhs), Value::Int32(rhs)) => Value::Int32(lhs << rhs),
            (Value::Int64(lhs), Value::Int64(rhs)) => Value::Int64(lhs << rhs),
            (Value::Uint8(lhs), Value::Uint8(rhs)) => Value::Uint8(lhs << rhs),
            (Value::Uint16(lhs), Value::Uint16(rhs)) => Value::Uint16(lhs << rhs),
            (Value::Uint32(lhs), Value::Uint32(rhs)) => Value::Uint32(lhs << rhs),
            (Value::UInt64(lhs), Value::UInt64(rhs)) => Value::UInt64(lhs << rhs),
            (Value::UInt64(lhs), Value::Int64(rhs)) => Value::UInt64(lhs << rhs as u64),
            (Value::Int64(lhs), Value::UInt64(rhs)) => Value::UInt64((lhs as u64) << rhs),
            (Value::Int64(lhs), Value::Double(rhs)) => Value::Int64(lhs << (rhs as i64)),
            (Value::Double(lhs), Value::Int64(rhs)) => Value::Int64((lhs as i64) << rhs),
            (Value::UInt64(lhs), Value::Double(rhs)) => Value::UInt64(lhs << rhs as u64),
            (Value::Double(lhs), Value::UInt64(rhs)) => Value::UInt64((lhs as u64) << rhs),
            (lhs, rhs) => {
                panic!(
                    "Bitwise XOR not supported for types: {:?} and {:?}",
                    lhs, rhs
                );
            }
        }
    }
}

/// Bitwise shift right implementation for Value
impl Shr for Value {
    type Output = Self;

    /// Perform a bitwise shift right operation on two values
    ///
    /// # Arguments
    ///
    /// * `rhs` - The right hand side value
    ///
    /// # Returns
    ///
    /// * The result of the bitwise shift right operation
    fn shr(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Value::Int8(lhs), Value::Int8(rhs)) => Value::Int8(lhs >> rhs),
            (Value::Int16(lhs), Value::Int16(rhs)) => Value::Int16(lhs >> rhs),
            (Value::Int32(lhs), Value::Int32(rhs)) => Value::Int32(lhs >> rhs),
            (Value::Int64(lhs), Value::Int64(rhs)) => Value::Int64(lhs >> rhs),
            (Value::Uint8(lhs), Value::Uint8(rhs)) => Value::Uint8(lhs >> rhs),
            (Value::Uint16(lhs), Value::Uint16(rhs)) => Value::Uint16(lhs >> rhs),
            (Value::Uint32(lhs), Value::Uint32(rhs)) => Value::Uint32(lhs >> rhs),
            (Value::UInt64(lhs), Value::UInt64(rhs)) => Value::UInt64(lhs >> rhs),
            (Value::UInt64(lhs), Value::Int64(rhs)) => Value::UInt64(lhs >> rhs as u64),
            (Value::Int64(lhs), Value::UInt64(rhs)) => Value::UInt64((lhs as u64) >> rhs),
            (Value::Int64(lhs), Value::Double(rhs)) => Value::Int64(lhs >> (rhs as i64)),
            (Value::Double(lhs), Value::Int64(rhs)) => Value::Int64((lhs as i64) >> rhs),
            (Value::UInt64(lhs), Value::Double(rhs)) => Value::UInt64(lhs >> rhs as u64),
            (Value::Double(lhs), Value::UInt64(rhs)) => Value::UInt64((lhs as u64) >> rhs),
            (lhs, rhs) => {
                panic!(
                    "Bitwise XOR not supported for types: {:?} and {:?}",
                    lhs, rhs
                );
            }
        }
    }
}

/// Negation implementation for Value
impl Neg for Value {
    type Output = Self;

    /// Negate a value
    ///
    /// # Returns
    ///
    /// * The negated value
    fn neg(self) -> Self::Output {
        match self {
            Value::Int8(v) => Value::Int8(-v),
            Value::Int16(v) => Value::Int16(-v),
            Value::Int32(v) => Value::Int32(-v),
            Value::Int64(v) => Value::Int64(-v),
            Value::Uint8(v) => Value::Int8(-(v as i8)),
            Value::Uint16(v) => Value::Int16(-(v as i16)),
            Value::Uint32(v) => Value::Int32(-(v as i32)),
            Value::UInt64(v) => Value::Int64(-(v as i64)),
            Value::Float(v) => Value::Float(-v),
            Value::Double(v) => Value::Double(-v),
            _ => {
                panic!("Negation not supported for type: {:?}", self)
            }
        }
    }
}


/// Logical NOT implementation for Value
impl Not for Value {
    type Output = Self;

    /// Perform a logical NOT operation on a value
    ///
    /// # Returns
    ///
    /// * The result of the logical NOT operation
    fn not(self) -> Self::Output {
        match self {
            Value::Bool(v) => Value::Bool(!v),
            Value::Int8(v) => Value::Int8(!v),
            Value::Int16(v) => Value::Int16(!v),
            Value::Int32(v) => Value::Int32(!v),
            Value::Int64(v) => Value::Int64(!v),
            Value::Uint8(v) => Value::Int8(!(v as i8)),
            Value::Uint16(v) => Value::Int16(!(v as i16)),
            Value::Uint32(v) => Value::Int32(!(v as i32)),
            Value::UInt64(v) => Value::Int64(!(v as i64)),
            _ => {
                panic!("Negation not supported for type: {:?}", self)
            }
        }
    }
}

/// Value implementation
impl Value {

    /// Checks if the value is a primitive
    ///
    /// # Returns
    ///
    /// * True if the value is a primitive, else false
    pub fn is_referenced(&self) -> bool {
        match self {
            Value::ReferencedValue { .. } => true,
            _ => false,
        }
    }

    /// Converts the value to 8-bit integer
    ///
    /// # Returns
    ///
    /// * The 8-bit integer value
    pub fn to_i8(&self) -> i8 {
        match self {
            Value::Int8(v) => *v,
            Value::Int16(v) => *v as i8,
            Value::Int32(v) => *v as i8,
            Value::Int64(v) => *v as i8,
            Value::Uint8(v) => *v as i8,
            Value::Uint16(v) => *v as i8,
            Value::Uint32(v) => *v as i8,
            Value::UInt64(v) => *v as i8,
            Value::Float(v) => *v as i8,
            Value::Double(v) => *v as i8,
            _ => {
                panic!("Cannot convert value to i8: {:?}", self)
            }
        }
    }

    /// Converts the value to 16-bit integer
    ///
    /// # Returns
    ///
    /// * The 16-bit integer value
    pub fn to_i16(&self) -> i16 {
        match self {
            Value::Int8(v) => *v as i16,
            Value::Int16(v) => *v,
            Value::Int32(v) => *v as i16,
            Value::Int64(v) => *v as i16,
            Value::Uint8(v) => *v as i16,
            Value::Uint16(v) => *v as i16,
            Value::Uint32(v) => *v as i16,
            Value::UInt64(v) => *v as i16,
            Value::Float(v) => *v as i16,
            Value::Double(v) => *v as i16,
            _ => {
                panic!("Cannot convert value to i16: {:?}", self)
            }
        }
    }

    /// Converts the value to 32-bit integer
    ///
    /// # Returns
    ///
    /// * The 32-bit integer value
    pub fn to_i32(&self) -> i32 {
        match self {
            Value::Int8(v) => *v as i32,
            Value::Int16(v) => *v as i32,
            Value::Int32(v) => *v,
            Value::Int64(v) => *v as i32,
            Value::Uint8(v) => *v as i32,
            Value::Uint16(v) => *v as i32,
            Value::Uint32(v) => *v as i32,
            Value::UInt64(v) => *v as i32,
            Value::Float(v) => *v as i32,
            Value::Double(v) => *v as i32,
            _ => {
                panic!("Cannot convert value to i32: {:?}", self)
            }
        }
    }

    /// Converts the value to 64-bit integer
    ///
    /// # Returns
    ///
    /// * The 64-bit integer value
    pub fn to_i64(&self) -> i64 {
        match self {
            Value::Int8(v) => *v as i64,
            Value::Int16(v) => *v as i64,
            Value::Int32(v) => *v as i64,
            Value::Int64(v) => *v,
            Value::Uint8(v) => *v as i64,
            Value::Uint16(v) => *v as i64,
            Value::Uint32(v) => *v as i64,
            Value::UInt64(v) => *v as i64,
            Value::Float(v) => *v as i64,
            Value::Double(v) => *v as i64,
            _ => {
                panic!("Cannot convert value to i64: {:?}", self)
            }
        }
    }

    /// Converts the value to 8-bit unsigned integer
    ///
    /// # Returns
    ///
    /// * The 8-bit unsigned integer value
    pub fn to_u8(&self) -> u8 {
        match self {
            Value::Int8(v) => *v as u8,
            Value::Int16(v) => *v as u8,
            Value::Int32(v) => *v as u8,
            Value::Int64(v) => *v as u8,
            Value::Uint8(v) => *v,
            Value::Uint16(v) => *v as u8,
            Value::Uint32(v) => *v as u8,
            Value::UInt64(v) => *v as u8,
            Value::Float(v) => *v as u8,
            Value::Double(v) => *v as u8,
            _ => {
                panic!("Cannot convert value to u8: {:?}", self)
            }
        }
    }

    /// Converts the value to 16-bit unsigned integer
    ///
    /// # Returns
    ///
    /// * The 16-bit unsigned integer value
    pub fn to_u16(&self) -> u16 {
        match self {
            Value::Int8(v) => *v as u16,
            Value::Int16(v) => *v as u16,
            Value::Int32(v) => *v as u16,
            Value::Int64(v) => *v as u16,
            Value::Uint8(v) => *v as u16,
            Value::Uint16(v) => *v,
            Value::Uint32(v) => *v as u16,
            Value::UInt64(v) => *v as u16,
            Value::Float(v) => *v as u16,
            Value::Double(v) => *v as u16,
            _ => {
                panic!("Cannot convert value to u16: {:?}", self)
            }
        }
    }

    /// Converts the value to 32-bit unsigned integer
    ///
    /// # Returns
    ///
    /// * The 32-bit unsigned integer value
    pub fn to_u32(&self) -> u32 {
        match self {
            Value::Int8(v) => *v as u32,
            Value::Int16(v) => *v as u32,
            Value::Int32(v) => *v as u32,
            Value::Int64(v) => *v as u32,
            Value::Uint8(v) => *v as u32,
            Value::Uint16(v) => *v as u32,
            Value::Uint32(v) => *v,
            Value::UInt64(v) => *v as u32,
            Value::Float(v) => *v as u32,
            Value::Double(v) => *v as u32,
            _ => {
                panic!("Cannot convert value to u32: {:?}", self)
            }
        }
    }

    /// Converts the value to 64-bit unsigned integer
    ///
    /// # Returns
    ///
    /// * The 64-bit unsigned integer value
    pub fn to_u64(&self) -> u64 {
        match self {
            Value::Int8(v) => *v as u64,
            Value::Int16(v) => *v as u64,
            Value::Int32(v) => *v as u64,
            Value::Int64(v) => *v as u64,
            Value::Uint8(v) => *v as u64,
            Value::Uint16(v) => *v as u64,
            Value::Uint32(v) => *v as u64,
            Value::UInt64(v) => *v,
            Value::Float(v) => *v as u64,
            Value::Double(v) => *v as u64,
            _ => {
                panic!("Cannot convert value to u64: {:?}", self)
            }
        }
    }

    /// Converts the value to 32-bit floating point number
    ///
    /// # Returns
    ///
    /// * The 32-bit floating point number value
    pub fn to_f32(&self) -> f32 {
        match self {
            Value::Int8(v) => *v as f32,
            Value::Int16(v) => *v as f32,
            Value::Int32(v) => *v as f32,
            Value::Int64(v) => *v as f32,
            Value::Uint8(v) => *v as f32,
            Value::Uint16(v) => *v as f32,
            Value::Uint32(v) => *v as f32,
            Value::UInt64(v) => *v as f32,
            Value::Float(v) => *v,
            Value::Double(v) => *v as f32,
            _ => {
                panic!("Cannot convert value to f32: {:?}", self)
            }
        }
    }

    /// Converts the value to 64-bit floating point number
    ///
    /// # Returns
    ///
    /// * The 64-bit floating point number value
    pub fn to_f64(self) -> f64 {
        match self {
            Value::Int8(v) => v as f64,
            Value::Int16(v) => v as f64,
            Value::Int32(v) => v as f64,
            Value::Int64(v) => v as f64,
            Value::Uint8(v) => v as f64,
            Value::Uint16(v) => v as f64,
            Value::Uint32(v) => v as f64,
            Value::UInt64(v) => v as f64,
            Value::Float(v) => v as f64,
            Value::Double(v) => v,
            _ => {
                panic!("Cannot convert value to f64: {:?}", self)
            }
        }
    }

    /// Checks if the value is a numeric type
    ///
    /// # Returns
    ///
    /// * True if the value is numeric, else false
    pub fn is_numeric(&self) -> bool {
        matches!(
            self,
            Value::Int8(_)
                | Value::Int16(_)
                | Value::Int32(_)
                | Value::Int64(_)
                | Value::Uint8(_)
                | Value::Uint16(_)
                | Value::Uint32(_)
                | Value::UInt64(_)
                | Value::Float(_)
                | Value::Double(_)
        )
    }

    /// Checks if the value is an integer type
    ///
    /// # Returns
    ///
    /// * True if the value is an integer, else false
    pub fn is_integer(&self) -> bool {
        matches!(
            self,
            Value::Int8(_)
                | Value::Int16(_)
                | Value::Int32(_)
                | Value::Int64(_)
                | Value::Uint8(_)
                | Value::Uint16(_)
                | Value::Uint32(_)
                | Value::UInt64(_)
        )
    }

    /// Determines the type of the current Value
    ///
    /// # Returns
    ///
    /// * The type of the value
    pub fn determine_type(&self) -> Type {
        match self {
            Value::Bool(_) => Type::Bool,
            Value::Int8(_) => Type::Int8,
            Value::Int16(_) => Type::Int16,
            Value::Int32(_) => Type::Int32,
            Value::Int64(_) => Type::Int64,
            Value::Uint8(_) => Type::UInt8,
            Value::Uint16(_) => Type::UInt16,
            Value::Uint32(_) => Type::UInt32,
            Value::UInt64(_) => Type::UInt64,
            Value::Float(_) => Type::Float,
            Value::Double(_) => Type::Double,
            Value::String(_) => Type::String,
            Value::ReferencedValue { value: inner, .. } => inner.determine_type(),
            _ => {
                panic!("Cannot determine type for value: {:?}", self)
            }
        }
    }
}
