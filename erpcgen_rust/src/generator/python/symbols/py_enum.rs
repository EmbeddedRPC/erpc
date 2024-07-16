/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use crate::generator::python::util::format_comments;
use crate::symbols::enum_definition::EnumDefinition;
use crate::symbols::enum_member::EnumMember;

pub struct PyEnum {
    pub name: Option<String>,
    pub members: Vec<PyEnumMember>,
    pub preceding_comment: String,
    pub trailing_comment: String,
}

impl PyEnum {
    pub(crate) fn from(definition: &EnumDefinition) -> Self {
        Self {
            name: definition.name.clone(),
            members: definition.members.iter().map(PyEnumMember::from).collect(),
            preceding_comment: format_comments(definition.doxygen_preceding_comment.clone()),
            trailing_comment: format_comments(definition.doxygen_trailing_comment.clone()),
        }
    }
}

pub struct PyEnumMember {
    pub name: String,
    pub value: i32,
    pub preceding_comment: String,
    pub trailing_comment: String,
}

impl PyEnumMember {
    pub(crate) fn from(enum_member: &EnumMember) -> Self {
        Self {
            name: enum_member.name.clone(),
            value: enum_member.value.to_i32(),
            preceding_comment: format_comments(enum_member.doxygen_preceding_comment.clone()),
            trailing_comment: format_comments(enum_member.doxygen_trailing_comment.clone()),
        }
    }
}

