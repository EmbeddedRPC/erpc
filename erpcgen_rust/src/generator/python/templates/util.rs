/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


use crate::generator::python::symbols::py_struct::PyStructDataMember;
use crate::symbols::types::Type;
use crate::symbols::value::Value;
use std::ops::Deref;
use std::rc::Rc;
use crate::generator::python::symbols::py_function::PyFunction;
use crate::symbols::param_direction::ParamDirection;

fn value_to_string_py(value: &Value) -> String {
    match value {
        Value::Bool(bool) => bool.to_string(),
        Value::Int8(n) => n.to_string(),
        Value::Int16(n) => n.to_string(),
        Value::Int32(n) => n.to_string(),
        Value::Int64(n) => n.to_string(),
        Value::Uint8(n) => n.to_string(),
        Value::Uint16(n) => n.to_string(),
        Value::Uint32(n) => n.to_string(),
        Value::UInt64(n) => n.to_string(),
        Value::Float(n) => n.to_string(),
        Value::Double(n) => n.to_string(),
        Value::String(s) => s.clone(),
        Value::ReferencedValue { name, prefix, .. } => if let Some(p) = prefix{ p.clone() + "." + name.as_str() } else { name.clone() },
        _ => "".to_string(),
    }
}

pub fn format_values_py(values: &[Value]) -> String {
    values
        .iter()
        .map(value_to_string_py)
        .collect::<Vec<String>>()
        .join(", ")
}

pub fn format_type_name_py(type_: Rc<Type>) -> String {
    match type_.deref() {
        Type::List { element_type } => format!(
            "list<{}>",
            format_type_name_py(element_type.clone())
        ),
        Type::Array {
            element_type,
            dimension_sizes,
        } => {
            let mut result = format_type_name_py(element_type.clone());
            for size in dimension_sizes {
                result.push_str(&format!("[{}]", size));
            }
            result
        }
        Type::TypeDef { name, .. } => name.clone(),
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
        Type::Struct { name, .. } => name.clone(),
        Type::Enum { name, .. } => name.clone(),
        Type::Union { name, .. } => name.clone(),
        _ => "".to_string(),
    }
}

pub fn format_class_init_py(members: &[PyStructDataMember]) -> String {
    members
        .iter()
        .filter_map(|m| {
            if m.is_length_for.is_none() && !m.type_.is_union() {
                Some(format!("{}=None", m.name))
            } else {
                None
            }
        })
        .collect::<Vec<String>>()
        .join(", ")
}

pub fn shed_array_dimension(array_type: Rc<Type>) -> Rc<Type> {
    if let Type::List { element_type } = array_type.deref() {
        return element_type.clone()
    } 
    if let Type::Array { element_type, dimension_sizes } = array_type.deref() {
        if dimension_sizes.len() > 1 {
            let tail = dimension_sizes.clone().split_off(1);
            Rc::new(Type::Array {
                element_type: element_type.clone(),
                dimension_sizes: tail,
            })
        } else {
            element_type.clone()
        }
    } else {
        array_type
    }
}

pub fn get_current_array_dim(array_type: Rc<Type>) -> usize {
    if let Type::Array { dimension_sizes, .. } = array_type.deref() {
        if !dimension_sizes.is_empty() {
            dimension_sizes[0]
        } else {
            0
        }
    } else {
        0
    }
}

pub fn optional_indent(indent: &bool) -> String {
    if *indent {
        "    ".to_string()
    } else {
        "".to_string()
    }
}

pub fn format_case_values_py(values: &[Value], discriminator: &String ) -> String {
    values.iter().map(|v| format!("{} == {}", discriminator, value_to_string_py(v))).collect::<Vec<String>>().join(" or ")
}

pub fn decode_type(py_type: Rc<Type>, name: &str, prefix: &str, discriminator: Option<String>, codec: &str, indent: &str, depth: usize) -> String {
    match py_type.as_ref() {
        Type::Struct { .. } => {
            format!(
                "{} = {}{}()._read({})",
                name,
                prefix,
                py_type.get_name(),
                codec
            )
        }
        Type::Union { ..} => {
            format!(
                "{},{} = {}{}()._read({})",
                name,
                discriminator.clone().unwrap_or_else(|| "discriminator".to_string()),
                prefix,
                py_type.get_name(),
                codec
            )
        }
        Type::Enum { .. } => {
            format!(
                "{} = {}.read_int32()",
                name,
                codec,
            )
        }
        Type::String => {
            format!(
                "{}{} = {}.read_string()",
                indent,
                name,
                codec,
            )
        }
        Type::Binary => {
            format!(
                "{}{} = {}.read_binary()",
                indent,
                name,
                codec,
            )
        }
        Type::Array { dimension_sizes, .. } => {
            let mut res = String::new();
            let current_dim = dimension_sizes[0];
            let next_ident = if dimension_sizes.len() > 1 { format!("{}    ", indent) } else { indent.to_string() };
            res += format!("{}{} = []\n", indent, name).as_str();
            res += format!("{}for _i{} in range({}):\n", indent, depth, current_dim).as_str();
            res += format!("{}{}\n", indent, decode_type(shed_array_dimension(py_type), format!("_v{}", depth).as_str(), "", None, codec, next_ident.as_str(), depth+1)).as_str();
            res += format!("    {}{}.append(_v{})", indent, name, depth).as_str();
            res
        }
        Type::List { element_type} => {
            let mut res = String::new();
            res += format!("{}_n{} = {}.start_read_list()\n", indent, depth, codec).as_str();
            res += format!("{}{} = []\n", indent, name).as_str();
            res += format!("{}for _i{} in range(_n{}):\n", indent, depth, depth).as_str();
            res += format!("{}{}\n", indent, decode_type(element_type.clone(), format!("_v{}", depth).as_str(), "", None,  codec, format!("{}    ", indent).as_str(), depth+1)).as_str();
            res += format!("    {}{}.append(_v{})", indent, name, depth).as_str();
            res
        }
        Type::Callback { name: cb_name, ..} => {
            format!(
                "{}{} = {}[{}.read_int8()]",
                indent,
                name,
                cb_name,
                codec
            )
        }
        _ => format!("{}{} = {}.read_{}()", indent, name, codec, py_type.get_name())
    }
}

pub fn encode_type(py_type: Rc<Type>, name: &str, prefix: &str, discriminator: Option<String>, codec: &str, indent: &str, depth: usize) -> String {
    match py_type.as_ref() {
        Type::Struct { .. } => {
            format!(
                "{}._write({})",
                name,
                codec
            )
        }
        Type::Union { ..} => {
            format!(
                "{}{}.write({}, {})",
                indent,
                name,
                codec,
                discriminator.clone().unwrap_or_else(|| "discriminator".to_string()),
            )
        }
        Type::Enum { .. } => {
            format!(
                "{}{}.write_int32({})",
                indent,
                codec,
                name,
            )
        }
        Type::String => {
            format!(
                "{}{}.write_string({})",
                indent,
                codec,
                name,
            )
        }
        Type::Binary => {
            format!(
                "{}{}.write_binary({})",
                indent,
                codec,
                name,
            )
        }
        Type::Array { dimension_sizes, .. } => {
            let mut res = String::new();
            let next_ident = if dimension_sizes.len() > 1 { format!("{}    ", indent) } else { indent.to_string() };
            res += format!("{}for _i{} in {}:\n", indent, depth, name).as_str();
            res += format!("{}{}", indent, encode_type(shed_array_dimension(py_type), format!("_i{}", depth).as_str(), "", discriminator, codec, next_ident.as_str(), depth+1)).as_str();
            res
        }
        Type::List { element_type} => {
            let mut res = String::new();
            res += format!("{}{}.start_write_list(len({}))\n", indent, codec, name).as_str();
            res += format!("{}for _i{} in {}:\n", indent, depth, name).as_str();
            res += format!("{}{}", indent, encode_type(element_type.clone(), format!("_i{}", depth).as_str(), "", discriminator, codec, format!("{}    ", indent).as_str(), depth+1)).as_str();
            res
        }
        Type::Callback { name: cb_name, ..} => { // {$codec}.write_int8({$ info.tableName}.index({$name})){%>%}
            format!(
                "{}{}.write_int8({}.index({}))",
                indent,
                codec,
                cb_name,
                name
            )
        }
        _ => format!("{}{}.write_{}({})", indent, codec, py_type.get_name(), name)
    }
}

pub fn format_function_prototype_py(func: &PyFunction) -> String {
    let mut res = String::new();
    res += format!("{}(self, ", func.name).as_str();
    res += format_inner_parameters_py(func).as_str();
    res += ")";
    res
}

pub fn format_inner_parameters_py(func: &PyFunction) -> String {
    func.parameters.iter().filter_map(|p| {
        if p.is_length_for.is_some() {
            None
        } else {
            Some(p.name.clone())
        }
    } ).collect::<Vec<String>>().join(", ")
}

pub fn format_param_name(name: &str, param_direction: &ParamDirection) -> String {
    match param_direction {
        ParamDirection::In => name.to_string(),
        ParamDirection::Out => format!("{}.value", name),
        ParamDirection::InOut => format!("{}.value", name),
    }
}

pub fn remove_string_quotes(s: &str) -> String {
    s.replace('"', "")
}
