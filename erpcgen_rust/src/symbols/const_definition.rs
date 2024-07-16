/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use std::collections::HashMap;
use std::rc::Rc;

use pest::error::{Error, ErrorVariant};
use pest::iterators::Pair;

use crate::parser::grammar_parser::Rule;
use crate::symbols::annotations::Annotation;
use crate::symbols::doxygen_comment::DoxygenComment;
use crate::symbols::program::Program;
use crate::symbols::types::Type;
use crate::symbols::util::apply_rename_for_type;
use crate::symbols::value::Value;

/// A constant definition
#[derive(Debug, Clone)]
pub(crate) struct ConstDefinition{
    pub(crate) name: String,
    pub(crate) annotations: Vec<Annotation>,
    pub(crate) const_type: Rc<Type>,
    pub(crate) value: Value,
    pub(crate) doxygen_preceding_comment: Vec<DoxygenComment>,
    pub(crate) doxygen_trailing_comment: Vec<DoxygenComment>,
}

impl ConstDefinition {
    pub(crate) fn apply_rename(&mut self, renames: &HashMap<String, String>) {
        self.name = renames.get(&self.name).unwrap_or(&self.name).to_string();
        self.const_type = apply_rename_for_type(self.const_type.clone(), renames);
    }
}

/// A builder for a constant definition
pub(crate) struct ConstDefinitionBuilder<'a> {
    program: &'a Program,
    strict: bool,
    name: Option<String>,
    annotations: Vec<Annotation>,
    const_type: Option<Rc<Type>>,
    value: Option<Value>,
    doxygen_preceding_comment: Vec<DoxygenComment>,
    doxygen_trailing_comment: Vec<DoxygenComment>,
}

/// Implementation of the builder
impl<'a> ConstDefinitionBuilder<'a> {
    pub fn new(program: &'a Program, strict: bool) -> Self {
        Self {
            program,
            strict,
            name: None,
            annotations: Vec::new(),
            const_type: None,
            value: None,
            doxygen_preceding_comment: vec![],
            doxygen_trailing_comment: vec![],
        }
    }

    /// Add annotations to the constant definition
    ///
    /// # Arguments
    ///
    /// * `comment` - The annotations to add
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn with_doxygen_preceding_comment(mut self, comment: Vec<DoxygenComment>) -> Self {
        self.doxygen_preceding_comment = comment;
        self
    }

    /// Add annotations to the constant definition
    ///
    /// # Arguments
    ///
    /// * `comment` - The annotations to add
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn with_doxygen_trailing_comment(mut self, comment: Vec<DoxygenComment>) -> Self {
        self.doxygen_trailing_comment = comment;
        self
    }   

    /// Add annotations to the constant definition
    ///
    /// # Arguments
    ///
    /// * `rule` - The annotations to add
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn with_annotations(self, rule: Pair<Rule>) -> Result<Self, Box<Error<Rule>>> {
        // There are no annotations currently supported for const definitions
        Ok(self)
    }

    /// Add annotations to the constant definition
    ///
    /// # Arguments
    ///
    /// * `type_rule` - The annotations to add
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn with_type(mut self, type_rule: Pair<Rule>) -> Result<Self, Box<Error<Rule>>> {
        let const_type = self.program.resolve_simple_data_type(type_rule)?;
        self.const_type = Some(const_type);
        Ok(self)
    }

    /// Sets the name of the constant definition.
    ///
    /// This method takes a `Pair<Rule>` as input, which represents the parsed name of the constant definition.
    /// It checks if the name is already in use in the program. If the name is already in use, it returns an error.
    /// If the name is not in use, it sets the name of the constant definition and returns the builder.
    ///
    /// # Arguments
    ///
    /// * `rule` - The parsed name of the constant definition. This is a `Pair<Rule>` because the name is parsed from the input using the Pest parsing library.
    ///
    /// # Returns
    ///
    /// A result containing either the builder (if the name was successfully set) or an error (if the name is already in use).
    pub(crate) fn with_name(mut self, rule: Pair<Rule>) -> Result<Self, Box<Error<Rule>>> {
        let location = rule.as_span();
        let name = rule.as_str().to_string();
        if !self.program.check_symbol_name_is_free(&name) {
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
    
    /// Sets the value of the constant definition.
    ///
    /// This method takes an expression as input and resolves it to a value that is set as the value of the constant definition.
    /// The type of the constant definition must be defined before the value can be set. If the type is not defined, an error is returned.
    /// The value is resolved using the `resolve_value_from_expression` method of the program. This method takes the expression, the type of the constant definition, and a boolean indicating whether the resolution should be strict as input, and returns the resolved value.
    ///
    /// # Arguments
    ///
    /// * `expression` - The expression that should be resolved to a value. This is a Pair<Rule> because the expression is parsed from the input using the Pest parsing library.
    ///
    /// # Returns
    ///
    /// A result containing either the builder (if the value was successfully set) or an error (if the type of the constant definition was not defined or if the value could not be resolved).
    pub fn with_value(mut self, expression: Pair<Rule>) -> Result<Self, Box<Error<Rule>>> {
        let span = expression.as_span();

        let Some(const_type) = self.const_type.clone() else {
            return Err(Box::new(Error::new_from_span(
                ErrorVariant::CustomError {
                    message: "Const type must be defined before value".to_string(),
                },
                span,
            )));
        };
        
        let value = self.program.resolve_value_from_expression(expression, const_type.as_ref(), self.strict)?;

        self.value = Some(value);
        Ok(self)
    }

    /// Build the constant definition
    ///
    /// # Returns
    ///
    /// * The constant definition
    pub(crate) fn build(self) -> ConstDefinition {
        ConstDefinition {
            name: self.name.unwrap(),
            annotations: self.annotations,
            const_type: self.const_type.unwrap(),
            value: self.value.unwrap(),
            doxygen_preceding_comment: self.doxygen_preceding_comment,
            doxygen_trailing_comment: self.doxygen_trailing_comment,
        }
    }
}