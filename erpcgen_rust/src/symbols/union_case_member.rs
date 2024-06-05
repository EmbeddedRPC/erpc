/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use std::collections::HashMap;
use std::rc::Rc;
use log::debug;
use pest::error::{Error};
use pest::iterators::Pair;
use crate::parser::grammar_parser::Rule;
use crate::parser::util::annotations::{find_name_annotation, has_discriminator_annotation, parse_array_annotations, parse_enum_annotations, parse_list_annotations, parse_struct_annotations, parse_union_annotations};
use crate::parser::util::struct_definition::{extract_options_list, extract_struct_member_options, parse_member_options};
use crate::symbols::annotations::Annotation;
use crate::symbols::doxygen_comment::DoxygenComment;
use crate::symbols::member_options::MemberOptions;
use crate::symbols::pending_ident::PendingIdent;
use crate::symbols::types::Type;
use crate::symbols::union_case::UnionCaseBuilder;
use crate::symbols::util::{apply_rename_for_annotation, apply_rename_for_type};

/// Union case member
#[derive(Debug, PartialEq, Clone)]
pub(crate) struct UnionCaseMember {
    pub(crate) member_options: Vec<MemberOptions>,
    pub(crate) name: String,
    pub(crate) member_type: Rc<Type>,
    pub(crate) annotations: Vec<Annotation>,
    pub(crate) doxygen_preceding_comment: Vec<DoxygenComment>,
    pub(crate) doxygen_trailing_comment: Vec<DoxygenComment>,
}

impl UnionCaseMember {
    pub(crate) fn apply_rename(&mut self, union_renames: &HashMap<String, String>, program_renames: &HashMap<String, String>) {
        self.name = union_renames.get(&self.name).unwrap_or(&self.name).to_string();
        self.member_type = apply_rename_for_type(self.member_type.clone(), program_renames);
        self.annotations = self.annotations.iter().map(|a| apply_rename_for_annotation(a.clone(), union_renames, program_renames)).collect();
    
    }
}

/// Union case member builder
pub(crate) struct UnionCaseMemberBuilder<'a> {
    union_case_builder: &'a UnionCaseBuilder<'a>,
    strict: bool,
    member_options: Vec<MemberOptions>,
    name: Option<String>,
    member_type: Option<Rc<Type>>,
    annotations: Vec<Annotation>,
    doxygen_preceding_comment: Vec<DoxygenComment>,
    doxygen_trailing_comment: Vec<DoxygenComment>,
    pending_idents: Vec<PendingIdent>
}

/// Implementation of UnionCaseMemberBuilder
impl<'a> UnionCaseMemberBuilder<'a> {
    pub(super) fn new(union_case_builder: &'a UnionCaseBuilder<'a>, strict: bool) -> Self {
        Self {
            union_case_builder,
            strict,
            member_options: vec![],
            name: None,
            member_type: Some(Rc::new(Type::Int32)),
            annotations: vec![],
            doxygen_preceding_comment: vec![],
            doxygen_trailing_comment: vec![],
            pending_idents: vec![]
        }
    }

    /// Create a new UnionCaseMemberBuilder
    ///
    /// # Arguments
    ///
    /// * `rule` - Pair<Rule>
    ///
    /// # Returns
    ///
    /// * The builder.
    pub(crate) fn with_name(mut self, rule: Pair<Rule>) -> Result<Self, Box<Error<Rule>>> {
        let location = rule.as_span();
        let name = rule.as_str().to_string();
        debug!("With name {}", name);
        if !self.union_case_builder.union_builder.check_union_case_member_name_is_free(&name) {
            return Err(Box::new(Error::new_from_span(
                pest::error::ErrorVariant::CustomError {
                    message: format!("Symbol name {} is already in use", name).to_string(),
                },
                location,
            )));
        }
        self.name = Some(name);
        Ok(self)
    }

    /// Create a new UnionCaseMemberBuilder
    ///
    /// # Arguments
    ///
    /// * `rule` - Pair<Rule>
    ///
    /// # Returns
    ///
    /// * The builder.
    ///
    /// # Errors
    ///
    /// * Type must be defined
    /// * Union member must have a discriminator annotation
    pub(crate) fn with_annotations(mut self, rule: Pair<Rule>) -> Result<Self, Box<Error<Rule>>> {
        let cloned_rule = rule.clone();
        self.annotations = match self.member_type.clone().expect("Type must be defined").as_ref() {
            Type::Struct { .. } => parse_struct_annotations(rule, &self.union_case_builder.union_builder.program.language, self.strict.clone())?,
            Type::Enum { .. } => parse_enum_annotations(rule, &self.union_case_builder.union_builder.program.language, self.strict.clone())?,
            Type::Union { .. } => {
                let (ann, pend) = parse_union_annotations(rule,&self.union_case_builder.union_builder.program.language , self.strict.clone())?;
                if !has_discriminator_annotation(&ann) {
                    return Err(Box::new(Error::new_from_pos(
                        pest::error::ErrorVariant::CustomError {
                            message: "Union member must have a discriminator annotation".to_string(),
                        },
                        cloned_rule.as_span().start_pos(),
                    )));
                }
                self.pending_idents.extend(pend);
                ann
            },
            Type::Array { .. } => parse_array_annotations(rule, &self.union_case_builder.union_builder.program.language, self.strict.clone())?,
            Type::List { .. } => {
                let (ann, pend) = parse_list_annotations(&self.union_case_builder.union_builder.program, rule, &self.union_case_builder.union_builder.program.language, self.strict.clone())?;
                self.pending_idents.extend(pend);
                ann
            },
            _ => vec![]
        };
        if self.member_type.clone().is_some_and(|t| t.is_union()) && !has_discriminator_annotation(&self.annotations) {
            return Err(Box::new(Error::new_from_pos(
                pest::error::ErrorVariant::CustomError {
                    message: "Union member must have a discriminator annotation".to_string(),
                },
                cloned_rule.as_span().start_pos(),
            )));
        }
        if let Some(Annotation::Name {name, ..}) = find_name_annotation(&self.annotations) {
            self.name = Some(name.clone()); // Consider adding a check for name conflict
        }
        Ok(self)
    }

    /// Create a new UnionCaseMemberBuilder
    ///
    /// # Arguments
    ///
    /// * `comment` - Vec<DoxygenComment>
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn with_doxygen_preceding_comment(mut self, comment: Vec<DoxygenComment>) -> Self {
        self.doxygen_preceding_comment = comment;
        self
    }

    /// Create a new UnionCaseMemberBuilder
    ///
    /// # Arguments
    ///
    /// * `comment` - Vec<DoxygenComment>
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn with_doxygen_trailing_comment(mut self, comment: Vec<DoxygenComment>) -> Self {
        self.doxygen_trailing_comment = comment;
        self
    }


    /// Create a new UnionCaseMemberBuilder
    ///
    /// # Arguments
    ///
    /// * `rule` - Pair<Rule>
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn with_options(mut self, mut rule: Pair<Rule>) -> Result<Self, Box<Error<Rule>>>{
        let rules = rule.into_inner();
        let option_rules = extract_options_list(&rules);
        let member_options = match option_rules {
            Some(options) => {
                parse_member_options(options.into_inner(), self.strict)?
            }
            None => {
                vec![]
            }
        };
        self.member_options = member_options;
        Ok(self)
    }


    /// Create a new UnionCaseMemberBuilder
    ///
    /// # Arguments
    ///
    /// * `rule` - Pair<Rule>
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn with_type(mut self, rule: Pair<Rule>) -> Result<Self, Box<Error<Rule>>> {
        let member_type = self.union_case_builder.union_builder.program.resolve_simple_data_type(rule)?;
        self.member_type = Some(member_type);
        Ok(self)
    }


    /// Create a new UnionCaseMemberBuilder
    ///
    /// # Returns
    ///
    /// * The UnionCaseMember, and a vector of PendingIdent
    ///
    /// # Panics
    ///
    /// * If the UnionCaseMember name is missing
    /// * If the UnionCaseMember type is missing
    pub(crate) fn build(self) -> (UnionCaseMember, Vec<PendingIdent>) {
        (UnionCaseMember {
            member_options: self.member_options,
            name: self.name.expect("Union case member name is missing"),
            member_type: self.member_type.expect("Union case member type is missing"),
            annotations: self.annotations,
            doxygen_preceding_comment: self.doxygen_preceding_comment,
            doxygen_trailing_comment: self.doxygen_trailing_comment,
        }, self.pending_idents)
    }
}