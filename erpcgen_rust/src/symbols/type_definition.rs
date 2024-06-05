/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use std::collections::HashMap;
use std::hash::Hash;
use std::rc::Rc;
use pest::error::{Error, ErrorVariant};
use pest::iterators::Pair;
use crate::parser::grammar_parser::Rule;
use crate::parser::util::annotations::parse_typedef_annotations;
use crate::symbols::annotations::Annotation;
use crate::symbols::doxygen_comment::DoxygenComment;
use crate::symbols::program::Program;
use crate::symbols::types::Type;
use crate::symbols::util::apply_rename_for_type;

/// Type definition
#[derive(Debug, Clone, PartialEq)]
pub(crate) struct TypeDefinition {
    pub(crate) name: String,
    pub(crate) annotations: Vec<Annotation>,
    pub(crate) doxygen_preceding_comment: Vec<DoxygenComment>,
    pub(crate) doxygen_trailing_comment: Vec<DoxygenComment>,
    pub(crate) referenced_type: Rc<Type>
}

/// Hash implementation for TypeDefinition
impl Hash for TypeDefinition {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        self.name.hash(state);
    }
}

/// Equality implementation for TypeDefinition
impl Eq for TypeDefinition {}

/// Implementation of TypeDefinition

impl TypeDefinition {
    pub fn apply_rename(&mut self, renames: &HashMap<String, String>) {
        self.name = renames.get(&self.name).unwrap_or(&self.name).to_string();
        self.referenced_type = apply_rename_for_type(self.referenced_type.clone(), renames);
    }
}

/// Type definition builder
pub(crate) struct TypeDefinitionBuilder<'a> {
    pub(super) program: &'a Program,
    strict: bool,
    name: Option<String>,
    annotations: Vec<Annotation>,
    doxygen_preceding_comment: Vec<DoxygenComment>,
    doxygen_trailing_comment: Vec<DoxygenComment>,
    referenced_type: Option<Rc<Type>>,
}

/// Implementation of TypeDefinitionBuilder
impl<'a> TypeDefinitionBuilder<'a> {
    pub fn new(program: &'a Program, strict: bool) -> Self {
        Self {
            program,
            strict,
            name: None,
            annotations: vec![],
            doxygen_preceding_comment: vec![],
            doxygen_trailing_comment: vec![],
            referenced_type: None,
        }
    }

    /// Create a new TypeDefinitionBuilder
    ///
    /// # Arguments
    ///
    /// * `comment` - A vector of DoxygenComment
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn with_doxygen_preceding_comment(mut self, comment: Vec<DoxygenComment>) -> Self {
        self.doxygen_preceding_comment = comment;
        self
    }

    /// Create a new TypeDefinitionBuilder
    ///
    /// # Arguments
    ///
    /// * `comment` - A vector of DoxygenComment
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn with_doxygen_trailing_comment(mut self, comment: Vec<DoxygenComment>) -> Self {
        self.doxygen_trailing_comment = comment;
        self
    }

    /// Create a new TypeDefinitionBuilder
    ///
    /// # Arguments
    ///
    /// * `rule` - Pair<Rule>
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn with_annotations(mut self, rule: Pair<Rule>) -> Result<Self, Box<Error<Rule>>> {
        self.annotations = parse_typedef_annotations(rule, &self.program.language, self.strict)?;
        Ok(self)
    }

    /// Create a new TypeDefinitionBuilder
    ///
    /// # Arguments
    ///
    /// * `type_rule` - Pair<Rule>
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn with_simple_type(mut self, type_rule: Pair<Rule>) -> Result<Self ,Box<Error<Rule>>> {
        let referenced_type = self.program.resolve_simple_data_type(type_rule)?;
        self.referenced_type = Some(referenced_type);
        Ok(self)
    }

    /// Create a new TypeDefinitionBuilder
    ///
    /// # Returns
    ///
    /// * The builder
    ///
    /// # Errors
    ///
    /// * Struct definition must be defined before type definition
    pub(crate) fn with_latest_struct_definition(mut self) -> Result<Self, Box<Error<Rule>>> {
        let referenced_type = self.program.struct_definitions.last()
            .expect("Struct definition must be defined before type definition");
        self.referenced_type = self.program.resolve_type_from_ident(referenced_type.name.as_str());
        Ok(self)
    }

    /// Create a new TypeDefinitionBuilder
    ///
    /// # Returns
    ///
    /// * The builder
    ///
    /// # Errors
    ///
    /// * Enum definition must be defined before type definition
    /// * Type definition must have a name
    pub(crate) fn with_latest_enum_definition(mut self) -> Result<Self, Box<Error<Rule>>> {
        let referenced_type = self.program.enum_definitions.last()
            .expect("Enum definition must be defined before type definition");
        if let Some(name) = referenced_type.name.clone() {
            self.referenced_type = self.program.resolve_type_from_ident(name.as_str());
        } else {
            self.referenced_type = self.program.resolve_type_from_ident(self.name.clone().expect("Type definition must have a name").as_str());
        }
        Ok(self)
    }


    /// Create a new TypeDefinitionBuilder
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
    /// * Symbol name is already in use
    pub(crate) fn with_name(mut self, rule: Pair<Rule>) -> Result<Self, Box<Error<Rule>>> {
        let location = rule.as_span();
        let name = rule.as_str().to_string();
        if !self.program.check_value_name_is_free(&name)
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


    /// Create a new TypeDefinitionBuilder
    pub(crate) fn build(self) -> TypeDefinition {
        TypeDefinition {
            name: self.name.expect("Type definition must have a name"),
            annotations: self.annotations,
            doxygen_preceding_comment: self.doxygen_preceding_comment,
            doxygen_trailing_comment: self.doxygen_trailing_comment,
            referenced_type: self.referenced_type.expect("Type definition must have a type"),
        }
    }
}
        