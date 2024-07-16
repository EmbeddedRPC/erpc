/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use std::rc::Rc;
use crate::generator::python::symbols::py_union::PyUnionCase;
use crate::generator::python::util::{find_discriminator_name, format_comments, is_discriminator_for, is_length_for, reorder_cases_default_last};
use crate::generator::util::find_nullable_annotation;
use crate::symbols::struct_data_member::StructDataMember;
use crate::symbols::struct_definition::StructDefinition;
use crate::symbols::struct_member::StructMember;
use crate::symbols::types::Type;
use crate::symbols::union_definition::UnionDefinition;
use crate::symbols::value::Value;

pub struct PyStruct {
    pub name: String,
    pub members: Vec<PyStructMember>,
    pub unions: Vec<PyEncapsulatedUnion>,
    pub data_members: Vec<PyStructDataMember>,
    pub preceding_comment: String,
    pub trailing_comment: String,
}

impl PyStruct {
    pub(crate) fn from(definition: &StructDefinition) -> Self {
        let members: Vec<PyStructMember> = definition.members.iter().map(|m| PyStructMember::from(m, definition)).collect();

        Self {
            name: definition.name.clone(),
            members: members.clone(),
            unions: members.clone().into_iter().filter_map(|m| {
                if m.is_encapsulated_union() {
                    Some(m.to_encapsulated_union())
                } else {
                    None
                }
            }).collect(),
            data_members: members.into_iter().filter_map(|m| {
                if m.is_data_member() {
                    Some(m.to_data_member())
                } else {
                    None
                }
            }).collect(),
            preceding_comment: format_comments(definition.doxygen_preceding_comment.clone()),
            trailing_comment: format_comments(definition.doxygen_trailing_comment.clone()),
        }
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct PyStructDataMember {
    pub name: String,
    pub type_: Rc<Type>,
    pub is_length_for: Option<String>,
    pub is_discriminator: Option<String>,
    pub discriminator_name: Option<String>, 
    pub is_nullable: bool,
    pub preceding_comment: String,
    pub trailing_comment: String,
}

impl PyStructDataMember {
    pub(crate) fn from(struct_member: &StructDataMember, is_length_for: Option<String>, is_discriminator: Option<String>) -> Self {
        Self {
            name: struct_member.name.clone(),
            type_: struct_member.member_type.clone(),
            is_length_for,
            is_discriminator,
            discriminator_name: find_discriminator_name(&struct_member.annotations),
            is_nullable: find_nullable_annotation(&struct_member.annotations),
            preceding_comment: format_comments(struct_member.doxygen_preceding_comment.clone()),
            trailing_comment: format_comments(struct_member.doxygen_trailing_comment.clone()),
        }
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct PyEncapsulatedUnion {
    pub name: String,
    pub type_: Rc<Type>,
    pub cases: Vec<PyUnionCase>,
    pub discriminator_name: String,
    pub preceding_comment: String,
    pub trailing_comment: String,
}

impl PyEncapsulatedUnion {
    pub(crate) fn from(union: &UnionDefinition, discriminator: &Value) -> Self {
        let discriminator_name = match discriminator {
            Value::RuntimeValue { name } => name.clone(),
            _ => panic!("Unexpected discriminator value"),
        };
        
        Self {
            name: union.name.clone(),
            type_: Rc::new(Type::Union { name: union.name.clone() }),
            cases: reorder_cases_default_last(union.cases.iter().map(PyUnionCase::from).collect()),
            discriminator_name,
            preceding_comment: format_comments(union.doxygen_preceding_comment.clone()),
            trailing_comment: format_comments(union.doxygen_trailing_comment.clone()),
        }
    }
    
    pub fn has_default_case(&self) -> bool {
        self.cases.iter().any(|c| c.case_values.is_empty())
    }
    
    pub fn has_only_default_case(&self) -> bool {
        self.cases.iter().all(|c| c.case_values.is_empty())
    }
}

#[derive(Debug, Clone, PartialEq)]
pub enum PyStructMember {
    DataMember(PyStructDataMember),
    EncapsulatedUnion(PyEncapsulatedUnion),
}

impl PyStructMember {

    pub fn from(member: &StructMember, definition: &StructDefinition) -> Self {
        match member {
            StructMember::DataMember(d) => PyStructMember::DataMember(PyStructDataMember::from(d, is_length_for(definition, &d.name), is_discriminator_for(definition, &d.name))),
            StructMember::UnionDefinition { definition: u, discriminator: d } => PyStructMember::EncapsulatedUnion(PyEncapsulatedUnion::from(u, d)),
        }
    }
    pub fn is_data_member(&self) -> bool {
        matches!(self, PyStructMember::DataMember(_))
    }

    pub fn is_encapsulated_union(&self) -> bool {
        matches!(self, PyStructMember::EncapsulatedUnion(_))
    }

    pub fn to_data_member(self) -> PyStructDataMember {
        match self {
            PyStructMember::DataMember(d) => d,
            _ => panic!("Expected data member"),
        }
    }

    pub fn to_encapsulated_union(self) -> PyEncapsulatedUnion {
        match self {
            PyStructMember::EncapsulatedUnion(u) => u,
            _ => panic!("Expected encapsulated union"),
        }
    }
}

