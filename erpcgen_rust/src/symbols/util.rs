/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use std::collections::HashMap;
use std::ops::Deref;
use std::rc::Rc;
use crate::symbols::annotations::Annotation;
use crate::symbols::types::Type;
use crate::symbols::value::Value;

/// Applies the rename mappings to a given type.
///
/// This function takes a type and a HashMap of renames as input, and returns a new type where the names have been replaced according to the rename mappings.
/// The rename mappings are applied to the name of the type itself (if applicable) and recursively to the names of any contained types (for composite types like arrays and lists).
/// If a type does not have a name or if its name is not in the rename mappings, it is returned unchanged.
///
/// # Arguments
///
/// * `type_` - The type to which the rename mappings should be applied. This is an Rc<Type> because types can be shared between different parts of the program.
/// * `renames` - The rename mappings. This is a reference to a HashMap where each key-value pair represents a rename mapping from an original name to a new name.
///
/// # Returns
///
/// A new type where the names have been replaced according to the rename mappings. This is returned as an Rc<Type> because types can be shared between different parts of the program.
pub(crate) fn apply_rename_for_type(type_: Rc<Type>, renames: &HashMap<String, String>) -> Rc<Type> {
    match type_.deref() {
        Type::Enum { name, .. } => {
            let new_name = renames.get(name).unwrap_or(name).to_string();
            Rc::new(Type::Enum { name: new_name })
        }
        Type::Struct { name, .. } => {
            let new_name = renames.get(name).unwrap_or(name).to_string();
            Rc::new(Type::Struct { name: new_name })
        }
        Type::Union { name, .. } => {
            let new_name = renames.get(name).unwrap_or(name).to_string();
            Rc::new(Type::Union { name: new_name })
        }
        Type::Array { element_type, dimension_sizes } => {
            let new_element_type = apply_rename_for_type(element_type.clone(), renames);
            Rc::new(Type::Array { element_type: new_element_type, dimension_sizes: dimension_sizes.clone() })
        }
        Type::List { element_type } => {
            let new_element_type = apply_rename_for_type(element_type.clone(), renames);
            Rc::new(Type::List { element_type: new_element_type })
        }
        Type::TypeDef { name, referenced_type } => {
            let new_name = renames.get(name).unwrap_or(name).to_string();
            let new_referenced_type = apply_rename_for_type(referenced_type.clone(), renames);
            Rc::new(Type::TypeDef { name: new_name, referenced_type: new_referenced_type })
        }
        Type::Callback { scope, name } => {
            let new_scope = renames.get(scope).unwrap_or(scope).to_string();
            let new_name = renames.get(name).unwrap_or(name).to_string();
            Rc::new(Type::Callback { scope: new_scope, name: new_name })
        }
        _ => type_.clone()
    }
}

/// Applies the rename mappings to a given annotation.
///
/// This function takes an annotation and two HashMaps of renames as input, and returns a new annotation where the names have been replaced according to the rename mappings.
/// The rename mappings are applied to the name of the value in the annotation (if applicable).
/// If a value does not have a name or if its name is not in the rename mappings, it is returned unchanged.
///
/// # Arguments
///
/// * `annotation` - The annotation to which the rename mappings should be applied.
/// * `member_renames` - The rename mappings for members. This is a reference to a HashMap where each key-value pair represents a rename mapping from an original name to a new name.
/// * `program_renames` - The rename mappings for the program. This is a reference to a HashMap where each key-value pair represents a rename mapping from an original name to a new name.
///
/// # Returns
///
/// A new annotation where the names have been replaced according to the rename mappings.
pub fn apply_rename_for_annotation(annotation: Annotation, member_renames: &HashMap<String, String>, program_renames: &HashMap<String, String>,) -> Annotation {
    match annotation {
        Annotation::Discriminator { value, language_specifier } => {
            if let Value::RuntimeValue { name } = value {
                let new_name = member_renames.get(&name).unwrap_or(&name).to_string();
                Annotation::Discriminator { value: Value::RuntimeValue { name: new_name }, language_specifier }
            } else if let Value::ReferencedValue { prefix, name,  value} = value {
                let new_name = program_renames.get(&name).unwrap_or(&name).to_string();
                let new_prefix = prefix.map(|p| program_renames.get(&p).unwrap_or(&p).to_string());
                Annotation::Discriminator { value: Value::ReferencedValue { name: new_name, value, prefix: new_prefix }, language_specifier }
            } else {
                Annotation::Discriminator { value, language_specifier }
            }
        }
        Annotation::Length { length, language_specifier } => {
            if let Value::RuntimeValue { name } = length {
                let new_name = member_renames.get(&name).unwrap_or(&name).to_string();
                Annotation::Length { length: Value::RuntimeValue { name: new_name }, language_specifier }
            } else if let Value::ReferencedValue { prefix, name,  value} = length {
                let new_name = program_renames.get(&name).unwrap_or(&name).to_string();
                let new_prefix = prefix.map(|p| program_renames.get(&p).unwrap_or(&p).to_string());
                Annotation::Length { length: Value::ReferencedValue { name: new_name, value, prefix: new_prefix }, language_specifier }
            } else {
                Annotation::Length { length, language_specifier }
            }
        }
        _ => annotation
    }
}