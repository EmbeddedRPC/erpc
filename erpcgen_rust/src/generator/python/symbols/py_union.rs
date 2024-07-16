/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use crate::generator::python::util::{
    find_discriminator_name, format_comments, is_discriminator_for_union_case_member,
    is_length_for_union_case_member, reorder_cases_default_last,
};
use crate::generator::util::find_nullable_annotation;
use crate::symbols::types::Type;
use crate::symbols::union_case::UnionCase;
use crate::symbols::union_case_member::UnionCaseMember;
use crate::symbols::union_definition::UnionDefinition;
use crate::symbols::value::Value;
use std::rc::Rc;
pub struct PyUnion {
    pub name: String,
    pub cases: Vec<PyUnionCase>,
    pub preceding_comment: String,
    pub trailing_comment: String,
}

impl PyUnion {
    pub(crate) fn from(definition: &UnionDefinition) -> Self {
        let mut cases: Vec<PyUnionCase> = definition.cases.iter().map(PyUnionCase::from).collect();
        for i in 0..cases.len() - 1 {
            if cases[i].case_members.is_empty() {
                let values = cases[i].clone().case_values.clone();
                cases[i + 1].case_values.extend(values);
                cases[i].case_values.clear();
            }
        }
        Self {
            name: definition.name.clone(),
            cases: reorder_cases_default_last(
                cases.into_iter().filter(|c| !c.case_members.is_empty()).collect(),
            ),
            preceding_comment: format_comments(definition.doxygen_preceding_comment.clone()),
            trailing_comment: format_comments(definition.doxygen_trailing_comment.clone()),
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
pub struct PyUnionCase {
    pub case_values: Vec<Value>,
    pub case_members: Vec<PyUnionCaseMember>,
}

impl PyUnionCase {
    pub(crate) fn from(union_case: &UnionCase) -> Self {
        Self {
            case_values: union_case.case_values.clone(),
            case_members: union_case
                .members
                .iter()
                .map(|c| {
                    PyUnionCaseMember::from(
                        c,
                        is_length_for_union_case_member(union_case, &c.name),
                        is_discriminator_for_union_case_member(union_case, &c.name),
                    )
                })
                .collect(),
        }
    }
    
    pub fn is_default(&self) -> bool {
        self.case_values.is_empty()
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct PyUnionCaseMember {
    pub name: String,
    pub type_: Rc<Type>,
    pub is_length_for: Option<String>,
    pub is_discriminator: Option<String>,
    pub discriminator_name: Option<String>,
    pub is_nullable: bool,
    pub preceding_comment: String,
    pub trailing_comment: String,
}

impl PyUnionCaseMember {
    pub(crate) fn from(
        union_case_member: &UnionCaseMember,
        is_length_for: Option<String>,
        is_discriminator: Option<String>,
    ) -> Self {
        Self {
            name: union_case_member.name.clone(),
            type_: union_case_member.member_type.clone(),
            is_length_for,
            is_discriminator,
            discriminator_name: find_discriminator_name(&union_case_member.annotations),
            is_nullable: find_nullable_annotation(
                &union_case_member.annotations,
            ),
            preceding_comment: format_comments(union_case_member.doxygen_preceding_comment.clone()),
            trailing_comment: format_comments(union_case_member.doxygen_trailing_comment.clone()),
        }
    }
}
