/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use std::collections::HashMap;
use std::hash::{Hash, Hasher};
use log::{debug, trace};
use crate::parser::grammar_parser::Rule;
use crate::parser::util::annotations::{find_name_annotation, parse_enum_annotations};
use crate::symbols::annotations::Annotation;
use crate::symbols::enum_member::{EnumMember, EnumMemberBuilder};
use crate::symbols::program::Program;
use pest::error::Error;
use pest::iterators::Pair;
use crate::symbols::doxygen_comment::DoxygenComment;

/// An enumeration definition
#[derive(Debug, PartialEq, Clone)]
pub(crate) struct EnumDefinition {
    pub(crate) name: Option<String>,
    pub(crate) members: Vec<EnumMember>,
    pub(crate) annotations: Vec<Annotation>,
    pub(crate) doxygen_preceding_comment: Vec<DoxygenComment>,
    pub(crate) doxygen_trailing_comment: Vec<DoxygenComment>,
}


/// Implementation of the equality for the enumeration definition
impl Eq for EnumDefinition { }

/// Implementation of the enumeration definition
impl EnumDefinition {
    pub fn new(
        name: Option<String>,
        members: Vec<EnumMember>,
        annotations: Vec<Annotation>,
        doxygen_preceding_comment: Vec<DoxygenComment>,
        doxygen_trailing_comment: Vec<DoxygenComment>,
    ) -> Self {
        Self {
            name,
            members,
            annotations,
            doxygen_preceding_comment,
            doxygen_trailing_comment,
        }
    }
}

/// Hash implementation for the enumeration definition
impl Hash for EnumDefinition {
    fn hash<H: Hasher>(&self, state: &mut H) {
        self.name.hash(state);
    }
}

impl EnumDefinition {
    pub fn apply_rename(&mut self, renames: &HashMap<String, String>) {
        if let Some(name) = &self.name {
            self.name = Some(renames.get(name).unwrap_or(name).to_string());
        }
        self.members.iter_mut().for_each(|m| m.apply_rename(renames));
    }
}

/// A builder for an enumeration definition
pub(crate) struct EnumDefinitionBuilder<'a> {
    pub(super) program: &'a Program,
    strict: bool,
    name: Option<String>,
    pub(super) members: Vec<EnumMember>,
    annotations: Vec<Annotation>,
    doxygen_preceding_comment: Vec<DoxygenComment>,
    doxygen_trailing_comment: Vec<DoxygenComment>,
}

/// Implementation of the builder
impl<'a> EnumDefinitionBuilder<'a> {
    pub fn new(program: &'a Program, strict: bool) -> Self {
        Self {
            program,
            strict,
            name: None,
            members: vec![],
            annotations: vec![],
            doxygen_preceding_comment: vec![],
            doxygen_trailing_comment: vec![],
        }
    }

    /// Add a name to the enumeration definition
    ///
    /// # Arguments
    ///
    /// * `rule_opt` - The name to add
    ///
    /// # Returns
    ///
    /// * The builder
    ///
    /// # Errors
    ///
    /// * If the name is already in use
    pub(crate) fn with_name(mut self, rule_opt: Option<Pair<Rule>>) -> Result<Self, Box<Error<Rule>>> {
        // We look for the name in the annotations
        let name_from_annotation = find_name_annotation(&self.annotations);
        if let Some(Annotation::Name { name, ..}) = name_from_annotation {
            self.name = Some(name.clone());
            return Ok(self);
        }
        
        let Some(rule) = rule_opt else {
            return Ok(self)
        };
        let location = rule.as_span();
        let name = rule.as_str().to_string();
        debug!("With name: {}", name);
        if !self.program.check_symbol_name_is_free(&name) {
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

    /// Check if the enumeration member name is free
    ///
    /// # Arguments
    ///
    /// * `name` - The name to check
    ///
    /// # Returns
    ///
    /// * True if the name is free
    pub(super) fn check_enum_member_name_is_free(&self, name: &str) -> bool {
        self.members.iter().all(|member| member.name != name)
    }

    /// Add a member to the enumeration definition
    ///
    /// # Arguments
    ///
    /// * `member` - The member to add
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn add_member(mut self, member: EnumMember) -> Self {
        self.members.push(member); // We assume that has valid name from the builder of enum member
        self
    }

    /// Add annotations to the enumeration definition
    ///
    /// # Arguments
    ///
    /// * `annotations_list_rule` - The annotations to add
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn annotations(
        mut self,
        annotations_list_rule: Pair<Rule>,
    ) -> Result<Self, Box<Error<Rule>>> {
        self.annotations = parse_enum_annotations(annotations_list_rule, &self.program.language, self.strict)?;
        Ok(self)
    }

    /// Add doxygen preceding comment to the enumeration definition
    ///
    /// # Arguments
    ///
    /// * `comment` - The comment to add
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn with_doxygen_preceding_comment(mut self, comment: Vec<DoxygenComment>) -> Self {
        trace!("With doxygen preceding comment: {:?}", comment);
        self.doxygen_preceding_comment = comment;
        self
    }

    /// Add doxygen trailing comment to the enumeration definition
    ///
    /// # Arguments
    ///
    /// * `comment` - The comment to add
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn with_doxygen_trailing_comment(mut self, comment: Vec<DoxygenComment>) -> Self {
        trace!("With doxygen trailing comment: {:?}", comment);
        self.doxygen_trailing_comment = comment;
        self
    }

    /// Add an enumeration member
    ///
    /// # Arguments
    ///
    /// * `member` - The member to add
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn add_enum_member(&mut self, member: EnumMember) {
        self.members.push(member);
    }

    /// Create a new enumeration member builder
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn new_member_builder(&'a self) -> EnumMemberBuilder<'a> {
        EnumMemberBuilder::new(self, true)
    }

    /// Build the enumeration definition
    pub(crate) fn build(self) -> EnumDefinition {
        EnumDefinition::new(
            self.name,
            self.members,
            self.annotations,
            self.doxygen_preceding_comment,
            self.doxygen_trailing_comment,
        )
    }
}
