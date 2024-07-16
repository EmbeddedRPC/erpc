/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use std::rc::{Rc};

/// Type
#[derive(Debug, Clone, PartialEq, Hash)]
pub(crate) enum Type {
    Bool,
    Int8,
    Int16,
    Int32,
    Int64,
    UInt8,
    UInt16,
    UInt32,
    UInt64,
    Float,
    Double,
    String,
    Binary,
    Struct {
        name: String,
    },
    Enum {
        name: String
    },
    Union {
        name: String,
    },
    Array {
        element_type: Rc<Type>,
        dimension_sizes: Vec<usize>,
    },
    List {
        element_type: Rc<Type>,
    },
    TypeDef {
        name: String,
        referenced_type: Rc<Type>,
    },
    Callback {
        scope: String,
        name: String,
    },
}

/// Symbol type
pub(crate) enum SymbolType {
    Type,
    Struct,
    Enum,
    Union,
    Array,
    List,
    Callback,
}

/// Hash implementation for Type
impl Eq for Type {}

/// Type implementation
impl Type {
    /// Checks if the type is a primitive
    ///
    /// # Returns
    ///
    /// * True if the type is a primitive, else false
    pub(crate) fn is_primitive(&self) -> bool {
        if let Type::TypeDef { referenced_type, .. } = self {
            return referenced_type.is_primitive();
        }
        matches!(self, Type::Bool | Type::Int8 | Type::Int16 | Type::Int32 | Type::Int64 | Type::UInt8 | Type::UInt16 | Type::UInt32 | Type::UInt64 | Type::Float | Type::Double | Type::String)
    }


    /// Checks if the type is a struct
    ///
    /// # Returns
    ///
    /// * True if the type is a struct, else false
    pub(crate) fn is_struct(&self) -> bool {
        match self {
            Type::Struct { .. } => true,
            Type::TypeDef { referenced_type, .. } => {
                referenced_type.is_struct()
            },
            _ => false
        }
    }


    /// Checks if the type is an enum
    ///
    /// # Returns
    ///
    /// * True if the type is an enum, else false
    pub(crate) fn is_enum(&self) -> bool {
        match self {
            Type::Enum { .. } => true,
            Type::TypeDef { referenced_type, .. } => {
                referenced_type.is_enum()
            },
            _ => false
        }
    }

    /// Checks if the type is a union
    ///
    /// # Returns
    ///
    /// * True if the type is a union, else false
    pub(crate) fn is_union(&self) -> bool {
        match self {
            Type::Union { .. } => true,
            Type::TypeDef { referenced_type, .. } => {
                referenced_type.is_union()
            },
            _ => false
        }
    }

    /// Checks if the type is an array
    ///
    /// # Returns
    ///
    /// * True if the type is an array, else false
    pub(crate) fn is_array(&self) -> bool {
        match self {
            Type::Array { .. } => true,
            Type::TypeDef { referenced_type, .. } => {
                referenced_type.is_array()
            },
            _ => false
        }
    }

    /// Checks if the type is a list
    ///
    /// # Returns
    ///
    /// * True if the type is a list, else false
    pub(crate) fn is_list(&self) -> bool {
        match self {
            Type::List { .. } => true,
            Type::TypeDef { referenced_type, .. } => {
                referenced_type.is_list()
            },
            _ => false
        }
    }

    /// Checks if the type is a function
    ///
    /// # Returns
    ///
    /// * True if the type is a function, else false
    pub(crate) fn is_function(&self) -> bool {
        match self {
            Type::Callback { .. } => true,
            Type::TypeDef { referenced_type, .. } => {
                referenced_type.is_function()
            },
            _ => false
        }
    }

    /// Checks if the type is a typedef
    ///
    /// # Returns
    ///
    /// * True if the type is a typedef, else false
    pub(crate) fn is_typedef(&self) -> bool {
        matches!(self, Type::TypeDef { .. })
    }

    /// Resolve original type
    ///
    /// # Returns
    ///
    /// * The original type
    pub(crate) fn resolve_original_type(&self) -> Rc<Type> {
        match self {
            Type::TypeDef { referenced_type, .. } => {
                referenced_type.resolve_original_type()
            },
            _ => Rc::from(self.clone())
        }
    }

    /// Checks if the type is complex
    ///
    /// # Returns
    ///
    /// * True if the type is complex, else false
    pub(crate) fn is_complex(&self) -> bool {
        if let Type::TypeDef { referenced_type, .. } = self {
            return referenced_type.is_complex();
        }
        self.is_struct() || self.is_enum() || self.is_union() || self.is_array() || self.is_list()
    }

    /// Checks if the type is simple
    ///
    /// # Returns
    ///
    /// * True if the type is simple, else false
    pub(crate) fn is_simple(&self) -> bool {
        if let Type::TypeDef { referenced_type, .. } = self {
            return referenced_type.is_simple();
        }
        self.is_primitive()
    }

    /// Checks if the type is an integer
    ///
    /// # Returns
    ///
    /// * True if the type is an integer, else false
    pub(crate) fn is_integer(&self) -> bool {
        if let Type::TypeDef { referenced_type, .. } = self {
            return referenced_type.is_integer();
        }
        matches!(self, Type::Int8 | Type::Int16 | Type::Int32 | Type::Int64 | Type::UInt8 | Type::UInt16 | Type::UInt32 | Type::UInt64)
    }

    /// Checks if the type is a boolean
    ///
    /// # Returns
    ///
    /// * True if the type is a boolean, else false
    pub(crate) fn is_boolean(&self) -> bool {
        if let Type::TypeDef { referenced_type, .. } = self {
            return referenced_type.is_boolean();
        }
        matches!(self, Type::Bool)
    }

    /// Checks if the type is a scalar
    ///
    /// # Returns
    ///
    /// * True if the type is a scalar, else false
    pub(crate) fn is_scalar(&self) -> bool {
        if let Type::TypeDef { referenced_type, .. } = self {
            return referenced_type.is_scalar();
        }
        self.is_integer() || self.is_boolean() || self.is_enum()
    }

    /// Checks if the type is a floating point
    ///
    /// # Returns
    ///
    /// * True if the type is a floating point, else false
    pub(crate) fn is_string(&self) -> bool {
        if let Type::TypeDef { referenced_type, .. } = self {
            return referenced_type.is_string();
        }
        matches!(self, Type::String)
    }

    /// Checks if the type is binary
    ///
    /// # Returns
    ///
    /// * True if the type is binary, else false
    pub(crate) fn is_binary(&self) -> bool {
        if let Type::TypeDef { referenced_type, .. } = self {
            return referenced_type.is_binary();
        }
        matches!(self, Type::Binary)
    }

    /// Get the name of the type
    ///
    /// # Returns
    ///
    /// * The name of the type
    pub(crate) fn get_name(&self) -> String {
        match self {
            Type::Bool => "bool".to_string(),
            Type::Int8 => "int8".to_string(),
            Type::Int16 => "int16".to_string(),
            Type::Int32 => "int32".to_string(),
            Type::Int64 => "int64".to_string(),
            Type::UInt8 => "uint8".to_string(),
            Type::UInt16 => "uint16".to_string(),
            Type::UInt32 => "uint32".to_string(),
            Type::UInt64 => "uint64".to_string(),
            Type::Float => "float".to_string(),
            Type::Double => "double".to_string(),
            Type::String => "string".to_string(),
            Type::Binary => "binary".to_string(),
            Type::Struct { name } => name.clone(),
            Type::Enum { name } => name.clone(),
            Type::Union { name } => name.clone(),
            Type::Array { element_type, dimension_sizes } => {
                let mut result = element_type.get_name();
                for size in dimension_sizes {
                    result.push_str(&format!("[{}]", size));
                }
                result
            },
            Type::List { element_type } => format!("list<{}>", element_type.get_name()),
            Type::TypeDef { name, .. } => name.clone(),
            Type::Callback { scope, name } => format!("{}::{}", scope, name),
        }
    }

    /// Get the inner type
    ///
    /// # Returns
    ///
    /// * The inner type
    pub(crate) fn get_inner_type(&self) -> Rc<Type> {
        match self {
            Type::Array { element_type, .. } => element_type.clone(),
            Type::List { element_type } => element_type.clone(),
            Type::TypeDef { referenced_type, .. } => referenced_type.get_inner_type(),
            _ => Rc::from(self.clone())
        }
    }

    /// Resolve primitive type
    ///
    /// # Arguments
    ///
    /// * `ident` - The identifier
    ///
    /// # Returns
    ///
    /// * The type if found, else None
    pub(crate) fn resolve_primitive_type(ident: &str) -> Option<Type> {
        match ident {
            "bool" => Some(Type::Bool),
            "int8" => Some(Type::Int8),
            "int16" => Some(Type::Int16),
            "int32" => Some(Type::Int32),
            "int64" => Some(Type::Int64),
            "uint8" => Some(Type::UInt8),
            "uint16" => Some(Type::UInt16),
            "uint32" => Some(Type::UInt32),
            "uint64" => Some(Type::UInt64),
            "float" => Some(Type::Float),
            "double" => Some(Type::Double),
            "string" => Some(Type::String),
            "binary" => Some(Type::Binary),
            _ => None
        }
    }
}