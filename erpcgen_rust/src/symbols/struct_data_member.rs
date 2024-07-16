/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use std::collections::HashMap;
use std::rc::Rc;

use pest::error::Error;
use pest::iterators::Pair;

use crate::parser::grammar_parser::Rule;
use crate::parser::util::annotations::{find_name_annotation, has_discriminator_annotation, parse_array_annotations, parse_enum_annotations, parse_list_annotations, parse_member_annotations, parse_struct_annotations, parse_union_annotations};
use crate::parser::util::struct_definition::{extract_options_list, parse_member_options};
use crate::symbols::annotations::Annotation;
use crate::symbols::doxygen_comment::DoxygenComment;
use crate::symbols::member_options::MemberOptions;
use crate::symbols::pending_ident::PendingIdent;
use crate::symbols::struct_definition::StructDefinitionBuilder;
use crate::symbols::struct_member::StructMember;
use crate::symbols::types::Type;
use crate::symbols::util::{apply_rename_for_annotation, apply_rename_for_type};

/// Struct data member
#[derive(Debug, PartialEq, Clone)]
pub(crate) struct StructDataMember{
    pub(crate) member_options: Vec<MemberOptions>,
    pub(crate) name: String,
    pub(crate) member_type: Rc<Type>,
    pub(crate) annotations: Vec<Annotation>,
    pub(crate) doxygen_preceding_comment: Vec<DoxygenComment>,
    pub(crate) doxygen_trailing_comment: Vec<DoxygenComment>,
}

impl StructDataMember {
    pub fn apply_rename(&mut self, struct_renames: &HashMap<String, String>, program_renames: &HashMap<String, String>) {
        self.name = struct_renames.get(&self.name).unwrap_or(&self.name).to_string();
        self.member_type = apply_rename_for_type(self.member_type.clone(), program_renames);
        self.annotations = self.annotations.iter().cloned().map(|a| apply_rename_for_annotation(a, struct_renames, program_renames)).collect();
    }
}

/// Struct data member builder
pub(crate) struct StructDataMemberBuilder<'a> {
    struct_definition_builder: &'a StructDefinitionBuilder<'a>,
    strict: bool,
    member_options: Vec<MemberOptions>,
    name: Option<String>,
    member_type: Option<Rc<Type>>,
    annotations: Vec<Annotation>,
    doxygen_preceding_comment: Vec<DoxygenComment>,
    doxygen_trailing_comment: Vec<DoxygenComment>,
    pending_idents: Vec<PendingIdent>
}

/// Implementation of StructDataMemberBuilder
impl <'a> StructDataMemberBuilder<'a> {
    pub(super) fn new(struct_definition_builder: &'a StructDefinitionBuilder<'a>, strict: bool) -> Self {
        Self {
            struct_definition_builder,
            strict,
            member_options: vec![],
            name: None,
            member_type: None,
            annotations: vec![],
            doxygen_preceding_comment: vec![],
            doxygen_trailing_comment: vec![],
            pending_idents: vec![]
        }
    }

    /// Create a new StructDataMemberBuilder
    ///
    /// # Arguments
    ///
    /// * `rule` - Pair<Rule>
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn with_name(mut self, rule: Pair<Rule>) -> Result<Self, Box<Error<Rule>>> {
        let location = rule.as_span();
        let name = rule.as_str().to_string();
        if !self.struct_definition_builder.check_member_name_is_free(&name) {
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

    /// Create a new StructDataMemberBuilder
    ///
    /// # Arguments
    ///
    /// * `type_rule` - Pair<Rule>
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn with_type(mut self, type_rule: Pair<Rule>) -> Result<Self, Box<Error<Rule>>> {
        let member_type = self.struct_definition_builder.program.resolve_simple_data_type(type_rule)?;
        self.member_type = Some(member_type);
        Ok(self)
    }

    /// Create a new StructDataMemberBuilder
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

    /// Create a new StructDataMemberBuilder
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

    /// Create a new StructDataMemberBuilder
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


    /// Create a new StructDataMemberBuilder
    ///
    /// # Arguments
    ///
    /// * `rule` - Pair<Rule>
    ///
    /// # Returns
    ///
    /// * The builder
    ///
    /// # Errors
    ///
    /// * Type must be defined
    /// * Union member must have a discriminator annotation
    pub(crate) fn with_annotations(mut self, rule: Pair<Rule>) -> Result<Self, Box<Error<Rule>>> {
        self.annotations = match self.member_type.clone().expect("Type must be defined").as_ref() { 
            Type::Struct { .. } => parse_struct_annotations(rule.clone(), &self.struct_definition_builder.program.language, self.strict.clone())?,
            Type::Enum { .. } => parse_enum_annotations(rule.clone(), &self.struct_definition_builder.program.language, self.strict.clone())?,
            Type::Union { .. } => {
                let (ann, pend) = parse_union_annotations(rule.clone(),&self.struct_definition_builder.program.language , self.strict.clone())?;
                if !has_discriminator_annotation(&ann) {
                    return Err(Box::new(Error::new_from_pos(
                        pest::error::ErrorVariant::CustomError {
                            message: "Union member must have a discriminator annotation".to_string(),
                        },
                        rule.as_span().start_pos(),
                    )));
                }
                self.pending_idents.extend(pend);
                ann
            },
            Type::Array { .. } => parse_array_annotations(rule.clone(), &self.struct_definition_builder.program.language, self.strict)?,
            Type::List { .. } => {
                let (ann, pend) = parse_list_annotations(&self.struct_definition_builder.program, rule.clone(), &self.struct_definition_builder.program.language, self.strict.clone())?;
                self.pending_idents.extend(pend);
                ann
            },
            Type::Binary | Type::String => {
                let (ann, pend) = parse_list_annotations(self.struct_definition_builder.program, rule.clone(), &self.struct_definition_builder.program.language, self.strict)?;
                self.pending_idents.extend(pend);
                ann
            },
            _ => { 
                parse_member_annotations(rule.clone(), &self.struct_definition_builder.program.language, self.strict)?
            }
        };
        if self.member_type.clone().is_some_and(|t| t.is_union()) && !has_discriminator_annotation(&self.annotations) {
            return Err(Box::new(Error::new_from_pos(
                pest::error::ErrorVariant::CustomError {
                    message: "Union member must have a discriminator annotation".to_string(),
                },
                rule.as_span().start_pos(),
            )));
        }
        if let Some(Annotation::Name {name, ..}) = find_name_annotation(&self.annotations) {
            self.name = Some(name.clone()); // Consider adding a check for name conflict
        }
        Ok(self)
    }

    /// Build the StructDataMember
    pub(crate) fn build(self) -> (StructMember, Vec<PendingIdent>) {
        (StructMember::DataMember(StructDataMember {
                member_options: self.member_options,
                name: self.name.expect("Name is required for struct member"),
                member_type: self.member_type.expect("Type is required for struct member"),
                annotations: self.annotations,
                doxygen_preceding_comment: self.doxygen_preceding_comment,
                doxygen_trailing_comment: self.doxygen_trailing_comment,
        }), self.pending_idents)
    }
}