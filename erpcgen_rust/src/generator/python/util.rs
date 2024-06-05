use std::collections::{HashMap, HashSet};
use std::ops::Deref;
use std::rc::Rc;
use crate::generator::python::symbols::py_callback::PyCallback;
use crate::generator::python::symbols::py_union::PyUnionCase;
use crate::symbols::annotations::Annotation;
use crate::symbols::doxygen_comment::DoxygenComment;
use crate::symbols::function_definition::FunctionDefinition;
use crate::symbols::interface_definition::InterfaceDefinition;
use crate::symbols::struct_definition::StructDefinition;
use crate::symbols::struct_member::StructMember;
use crate::symbols::types::Type;
use crate::symbols::union_case::UnionCase;
use crate::symbols::value::Value;

/// Formats a list of doxygen comments into a single string
///
/// # Arguments
///
/// * `doxygen_comments` - A list of doxygen comments
///
/// # Returns
///
/// A string containing the formatted comments
pub(crate) fn format_comments(doxygen_comments: Vec<DoxygenComment>) -> String {
    let mut formatted_comments = String::new();
    for comment in doxygen_comments {
        match comment { 
            DoxygenComment::SingleLine(c) => {
                formatted_comments.push_str(&format!("# {}", c));
            },
            DoxygenComment::MultiLine(c) => {
                formatted_comments.push_str("'''\n");
                formatted_comments.push_str(&format!("{}\n", c));
                formatted_comments.push_str("'''\n");
            },
        }
    }
    formatted_comments
}

/// Formats a list of values into a single string
///
/// # Arguments
///
/// * `value` - A list of annotations
///
/// # Returns
///
/// A string containing the formatted values
pub(crate) fn value_to_string(value: Value) -> String {
    //use every type of value defined
    match value {
        Value::Bool(b) => b.to_string(),
        Value::Int8(i) => i.to_string(),
        Value::Int16(i) => i.to_string(),
        Value::Int32(i) => i.to_string(),
        Value::Int64(i) => i.to_string(),
        Value::Uint8(i) => i.to_string(),
        Value::Uint16(i) => i.to_string(),
        Value::Uint32(i) => i.to_string(),
        Value::UInt64(i) => i.to_string(),
        Value::Float(f) => f.to_string(),
        Value::Double(d) => d.to_string(),
        Value::Enum { value } => value.to_string(),
        Value::ReferencedValue { name, prefix, .. } => if let Some(p) = prefix { format!("{}.{}", p, name) } else { name },
        Value::String(s) => s,
        _ => String::new(), // EXTENSION POINT: Add more types
    }
}


/// Checks if a member is a length for a struct
///
/// # Arguments
///
/// * `struct_definition` - The struct definition
/// * `member_name` - The member name
///
/// # Returns
///
/// The name of the member that the length is for
pub(crate) fn is_length_for(struct_definition: &StructDefinition, member_name: &String) -> Option<String> {
    struct_definition.members.iter().find_map(|m| {
        if let StructMember::DataMember(d) = m {
            d.annotations.iter().find_map(|a| {
                match a {
                    Annotation::Length { length, .. } => if let Value::RuntimeValue { name } = length { 
                        if name == member_name {
                            Some(d.name.clone())
                        } else {
                            None
                        }
                    } else {
                        None
                    },
                    _ => None
                }
            })
        } else {
            None
        }
    })
}

/// Checks if a member is a length for a union case member
///
/// # Arguments
///
/// * `union_case_member` - The union case member
/// * `member_name` - The member name
///
/// # Returns
///
/// The name of the member that the length is for
pub fn is_length_for_union_case_member(union_case_member: &UnionCase, member_name: &String) -> Option<String> {
    union_case_member.members.iter().find_map(|m| {
        m.annotations.iter().find_map(|a| {
            match a {
                Annotation::Length { length, .. } => if let Value::RuntimeValue { name } = length {
                    if name == member_name {
                        Some(m.name.clone())
                    } else {
                        None
                    }
                } else {
                    None
                },
                _ => None
            }
        })
    })
}

/// Checks if a member is a length for a function
///
/// # Arguments
///
/// * `function` - The function definition
/// * `member_name` - The member name
///
/// # Returns
///
/// The name of the member that the length is for
pub fn is_length_for_function(function: &FunctionDefinition, member_name: &String) -> Option<String> {
    function.parameters.iter().find_map(|p| {
        p.annotations.iter().find_map(|a| {
            match a {
                Annotation::Length { length, .. } => if let Value::RuntimeValue { name } = length {
                    if name == member_name {
                        Some(p.name.clone().expect("Parameter name must be set"))
                    } else {
                        None
                    }
                } else {
                    None
                },
                _ => None
            }
        })
    })
}

/// Checks if a member is a discriminator for a struct
///
/// # Arguments
///
/// * `struct_definition` - The struct definition
/// * `member_name` - The member name
///
/// # Returns
///
/// The name of the member that the discriminator is for
pub(crate) fn is_discriminator_for(struct_definition: &StructDefinition, member_name: &String) -> Option<String> {
    // Look for encapsulated unions
    let res = struct_definition.members.iter().find_map(|m| { 
        if let StructMember::UnionDefinition { discriminator: Value::RuntimeValue { name}, .. } = m {
            if name == member_name {
                Some(name.clone())
            } else {
                None
            }
        } else {
            None
        }
    });
    
    if res.is_some() {
        return res;
    }
    // Look for unions
    struct_definition.members.iter().find_map(|m| {
        if let StructMember::DataMember(d) = m {
            d.annotations.iter().find_map(|a| {
                match a {
                    Annotation::Discriminator { value, .. } => if let Value::RuntimeValue { name } = value {
                        if name == member_name {
                            Some(name.clone())
                        } else {
                            None
                        }
                    } else {
                        None
                    },
                    _ => None
                }
            })
        } else {
            None
        }
    })    
}

/// Checks if a member is a discriminator for a union case member
///
/// # Arguments
///
/// * `union_case_member` - The union case member
/// * `member_name` - The member name
///
/// # Returns
///
/// The name of the member that the discriminator is for
pub(crate) fn is_discriminator_for_union_case_member(union_case_member: &UnionCase, member_name: &String) -> Option<String> {
    union_case_member.members.iter().find_map(|m| {
        m.annotations.iter().find_map(|a| {
            match a {
                Annotation::Discriminator { value, .. } => if let Value::RuntimeValue { name } = value {
                    if name == member_name {
                        Some(m.name.clone())
                    } else {
                        None
                    }
                } else {
                    None
                },
                _ => None
            }
        })
    })
}

/// Checks if a member is a discriminator for a function
///
/// # Arguments
///
/// * `function` - The function definition
/// * `member_name` - The member name
///
/// # Returns
///
/// The name of the member that the discriminator is for
pub(crate) fn is_discriminator_for_function(function: &FunctionDefinition, member_name: &String) -> Option<String> {
    function.parameters.iter().find_map(|p| {
        p.annotations.iter().find_map(|a| {
            match a {
                Annotation::Discriminator { value, .. } => if let Value::RuntimeValue { name } = value {
                    if name == member_name {
                        Some(p.name.clone().expect("Parameter name must be set"))
                    } else {
                        None
                    }
                } else {
                    None
                },
                _ => None
            }
        })
    })
}

/// Finds the name of the discriminator for a struct
///
/// # Arguments
///
/// * `annotations` - A list of annotations
///
/// # Returns
///
/// The name of the discriminator
pub(crate) fn find_discriminator_name(annotations: &[Annotation]) -> Option<String> {
    annotations.iter().find_map(|a| {
        match a {
            Annotation::Discriminator { value, .. } => if let Value::RuntimeValue {name} = value { 
                Some(name.clone())
            } else {
                None
            },
            _ => None
        }
    })
}

/// Finds includes in a list of interfaces and program annotations
///
/// # Arguments
///
/// * `interfaces` - A list of interfaces
/// * `program_annotations` - A list of annotations
///
/// # Returns
///
/// * A list of include paths
pub(crate) fn find_includes(interfaces: &Vec<InterfaceDefinition>, program_annotations: &Vec<Annotation>) -> Vec<String> {
    let mut includes = vec![];
    for interface in interfaces {
        for annotation in &interface.annotations {
            if let Annotation::Include { path, .. } = annotation {
                includes.push(path.clone());
            }
        }
    }
    for annotation in program_annotations {
        if let Annotation::Include { path, .. } = annotation {
            includes.push(path.clone());
        }
    }
    includes
}

/// Finds external annotations in a list of annotations
///
/// # Arguments
///
/// * `annotations` - A list of annotations
///
/// # Returns
///
/// * True if an external annotation is found, false otherwise
pub(crate) fn find_external_annotation(annotations: &[Annotation]) -> bool {
    annotations.iter().any(|a| {
        matches!(a, Annotation::External {..})
    })
}

/// Reorders cases so that the default case is last
///
/// # Arguments
///
/// * `list` - A list of union cases
///
/// # Returns
///
/// * A list of union cases with the default case last
pub(crate) fn reorder_cases_default_last(list: Vec<PyUnionCase>) -> Vec<PyUnionCase> {
    let default_case = list.iter().find(|c| c.case_values.is_empty()).cloned();
    let mut rest = list.iter().filter(|c| !c.case_values.is_empty()).cloned().collect::<Vec<PyUnionCase>>();
    if let Some(default_case) = default_case {
        rest.push(default_case);
    }
    rest
}

/// Finds used callbacks for a group
///
/// # Arguments
///
/// * `group_name` - The name of the group
/// * `interfaces` - A list of interfaces
/// * `groups` - A map of groups
///
/// # Returns
///
/// * A list of used callbacks
pub(crate) fn find_used_callbacks_for_group(group_name: &String, interfaces: &Vec<InterfaceDefinition>, groups: &HashMap<String, Vec<InterfaceDefinition>>) -> Vec<PyCallback> {
    let mut used_callbacks: HashSet<Rc<Type>> = HashSet::new();
    for interface in interfaces {
        for function in &interface.functions {
            if let Some(prototype) = &function.prototype {
                used_callbacks.insert(prototype.clone());
            }
            for parameter in &function.parameters {
                if let Type::Callback {..} = &parameter.type_.deref() {
                    used_callbacks.insert(parameter.type_.clone());
                }
            }
        }
    }
    
    let mut callbacks: Vec<PyCallback> = vec![];
    for callback in used_callbacks {
        let mut callback_functions: Vec<String> = vec![];
        for (name, interfaces) in groups  {
            for interface in interfaces {
                for function in &interface.functions {
                    if let Some(prototype) = &function.prototype {
                        if prototype == &callback {
                            if group_name == name { 
                                callback_functions.push(format!("interface.I{}Service.{}", interface.name, function.name));
                            } else {
                                callback_functions.push(format!("interface_{}.I{}Service.{}", group_name, interface.name, function.name));
                            }
                        }
                    }
                }
            }
        }
        let Type::Callback {name, ..} = &callback.deref() else { 
            panic!("Expected callback type")
        };
        callbacks.push(PyCallback::new(name, callback_functions));
    }
    callbacks
}
