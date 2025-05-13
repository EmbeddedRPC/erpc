/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use std::collections::HashMap;
use log::{debug, trace};
use crate::parser::grammar_parser::Rule;
use crate::parser::util::annotations::parse_enum_annotations;
use crate::parser::util::enum_definition::resolve_enum_member_value;
use crate::symbols::annotations::Annotation;
use crate::symbols::enum_definition::EnumDefinitionBuilder;
use crate::symbols::types::Type;
use crate::symbols::value::Value;
use pest::error::{Error, ErrorVariant};
use pest::iterators::Pair;
use crate::symbols::doxygen_comment::DoxygenComment;

/// An enumeration member
#[derive(Debug, PartialEq, Clone)]
pub(crate) struct EnumMember {
    pub(crate) name: String,
    pub(crate) value: Value,
    pub(crate) annotations: Vec<Annotation>,
    pub(crate) doxygen_preceding_comment: Vec<DoxygenComment>,
    pub(crate) doxygen_trailing_comment: Vec<DoxygenComment>,
}

/// A builder for an enumeration member
impl EnumMember {
    pub fn new(
        name: String,
        value: Value,
        annotations: Vec<Annotation>,
        doxygen_preceding_comment: Vec<DoxygenComment>,
        doxygen_trailing_comment: Vec<DoxygenComment>,
    ) -> Self {
        Self {
            name,
            value,
            annotations,
            doxygen_preceding_comment,
            doxygen_trailing_comment,
        }
    }
    
    pub fn apply_rename(&mut self, renames: &HashMap<String, String>) {
        self.name = renames.get(&self.name).unwrap_or(&self.name).to_string();
    }
}

/// A builder for an enumeration member
pub(crate) struct EnumMemberBuilder<'a> {
    enum_builder: &'a EnumDefinitionBuilder<'a>,
    strict: bool,
    name: Option<String>,
    value: Option<Value>,
    annotations: Vec<Annotation>,
    doxygen_preceding_comment: Vec<DoxygenComment>,
    doxygen_trailing_comment: Vec<DoxygenComment>,
}

/// Implementation of the builder
impl<'a> EnumMemberBuilder<'a> {
    pub(super) fn new(
        enum_builder: &'a EnumDefinitionBuilder<'a>,
        strict: bool,
    ) -> EnumMemberBuilder<'a> {
        Self {
            enum_builder,
            strict,
            name: None,
            value: None,
            annotations: vec![],
            doxygen_preceding_comment: vec![],
            doxygen_trailing_comment: vec![],
        }
    }

    /// Set the name of the enumeration member
    ///
    /// # Arguments
    ///
    /// * `rule` - The rule containing the name of the enumeration member
    ///
    /// # Returns
    ///
    /// * The enumeration member builder
    ///
    /// # Errors
    ///
    /// * If the name is already in use
    pub fn with_name(mut self, rule: Pair<Rule>) -> Result<Self, Box<Error<Rule>>> {
        let location = rule.as_span();
        let name = rule.as_str().to_string();
        debug!("With name: {}", name);
        if !self.enum_builder.program.check_symbol_name_is_free(&name)
            || !self.enum_builder.check_enum_member_name_is_free(&name)
        {
            return Err(Box::new(Error::new_from_span(
                ErrorVariant::CustomError {
                    message: format!("Symbol name {} is already in use", name).to_string(),
                },
                location,
            )));
        }

        self.name = Some(name);
        Ok(self)
    }

    /// Set the value of the enumeration member
    ///
    /// # Arguments
    ///
    /// * `expression` - The rule containing the value of the enumeration member
    ///
    /// # Returns
    ///
    /// * The enumeration member builder
    ///
    /// # Errors
    ///
    /// * If the value is not an integer
    pub fn with_value(mut self, expression: Pair<Rule>) -> Result<Self, Box<Error<Rule>>> {
        let span = expression.as_span();
        let value = self.enum_builder.program.resolve_value_from_expression(expression, &Type::Int32, false)?;
        debug!("With value: {:?}", value);
        if !value.is_integer() {
            return Err(Box::new(Error::new_from_span(
                ErrorVariant::CustomError {
                    message: "Enum member value must be an integer".to_string(),
                },
                span,
            )));
        }

        self.value = Some(value);
        Ok(self)
    }

    /// Add annotations to the enumeration member
    ///
    /// # Arguments
    ///
    /// * `annotations_list_rule` - The rule containing the annotations
    ///
    /// # Returns
    ///
    /// * The enumeration member builder
    pub(crate) fn annotations(
        mut self,
        annotations_list_rule: Pair<Rule>,
    ) -> Result<Self, Box<Error<Rule>>> {
        self.annotations = parse_enum_annotations(annotations_list_rule, &self.enum_builder.program.language, self.strict)?;
        Ok(self)
    }

    /// Add doxygen preceding comment to the enumeration member
    ///
    /// # Arguments
    ///
    /// * `comment` - The doxygen comment to add
    ///
    /// # Returns
    ///
    /// * The enumeration member builder
    pub(crate) fn with_doxygen_preceding_comment(mut self, comment: Vec<DoxygenComment>) -> Self {
        trace!("With doxygen preceding comment: {:?}", comment);
        self.doxygen_preceding_comment = comment;
        self
    }

    /// Add doxygen trailing comment to the enumeration member
    ///
    /// # Arguments
    ///
    /// * `comment` - The doxygen comment to add
    ///
    /// # Returns
    ///
    /// * The enumeration member builder
    pub(crate) fn with_doxygen_trailing_comment(mut self, comment: Vec<DoxygenComment>) -> Self {
        trace!("With doxygen trailing comment: {:?}", comment);
        self.doxygen_trailing_comment = comment;
        self
    }

    /// Build the enumeration member
    ///
    /// # Returns
    ///
    /// * The enumeration member
    ///
    /// # Panics
    ///
    /// * If the enumeration member name is not set
    pub fn build(self) -> EnumMember {
        let name = self.name.expect("Enum member name not set");
        let value = match self.value {
            Some(v) => v,
            None => {
                let member = self.enum_builder.members.iter().last();
                resolve_enum_member_value(member)
            }
        };
        EnumMember::new(
            name,
            value,
            self.annotations,
            self.doxygen_preceding_comment,
            self.doxygen_trailing_comment,
        )
    }
}
