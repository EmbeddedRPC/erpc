/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
use std::collections::HashMap;
use std::hash::{Hash, Hasher};
use pest::error::{Error, ErrorVariant};
use pest::iterators::Pair;
use crate::parser::grammar_parser::Rule;
use crate::parser::util::annotations::{find_name_annotation, parse_member_annotations, parse_union_annotations};
use crate::symbols::annotations::Annotation;
use crate::symbols::doxygen_comment::DoxygenComment;
use crate::symbols::program::Program;
use crate::symbols::struct_definition::StructDefinitionBuilder;
use crate::symbols::struct_member::StructMember;
use crate::symbols::types::Type;
use crate::symbols::union_case::{UnionCase, UnionCaseBuilder};
use crate::symbols::value::Value;

/// Union definition
#[derive(Debug, PartialEq, Clone)]
pub(crate) struct UnionDefinition {
    pub(crate) name: String,
    pub(crate) annotations: Vec<Annotation>,
    pub(crate) renames: HashMap<String, String>,
    pub(crate) cases: Vec<UnionCase>,
    pub(crate) doxygen_preceding_comment: Vec<DoxygenComment>,
    pub(crate) doxygen_trailing_comment: Vec<DoxygenComment>,
}

/// Hash implementation for UnionDefinition
impl Hash for UnionDefinition {
    fn hash<H: Hasher>(&self, state: &mut H) {
        self.name.hash(state);
    }
}

/// Equality implementation for UnionDefinition
impl Eq for UnionDefinition {}

impl UnionDefinition {
    pub fn apply_rename(&mut self, renames: &HashMap<String, String>) {
        self.name = renames.get(&self.name).unwrap_or(&self.name).to_string();
        self.cases.iter_mut().for_each(|c| c.apply_rename(&self.renames, renames));
    }
    
    pub fn apply_rename_encapsulated(&mut self, struct_renames: &HashMap<String, String>, renames: &HashMap<String, String>) {
        self.name = struct_renames.get(&self.name).unwrap_or(&self.name).to_string();
        self.cases.iter_mut().for_each(|c| c.apply_rename(&self.renames, renames));
    }
}

/// Union definition builder
pub(crate) struct UnionDefinitionBuilder<'a> {
    pub(super) program: &'a Program,
    pub(super) opt_struct_builder: Option<&'a StructDefinitionBuilder<'a>>,
    strict: bool,
    name: Option<String>,
    pub(super) has_default_case: bool,
    annotations: Vec<Annotation>,
    renames: HashMap<String, String>,
    doxygen_preceding_comment: Vec<DoxygenComment>,
    doxygen_trailing_comment: Vec<DoxygenComment>,
    cases: Vec<UnionCase>,
}

/// Implementation of UnionDefinitionBuilder
impl <'a>UnionDefinitionBuilder<'a> {
    pub fn new(program: &'a Program, strict: bool) -> Self {
        Self {
            program,
            opt_struct_builder: None,
            strict,
            name: None,
            has_default_case: false,
            annotations: vec![],
            renames: HashMap::new(),
            doxygen_preceding_comment: vec![],
            doxygen_trailing_comment: vec![],
            cases: vec![],
        }
    }

    /// Create a new UnionDefinitionBuilder
    ///
    /// # Arguments
    ///
    /// * `program` - A reference to the Program
    /// * `strict` - A boolean indicating if strict mode is enabled
    /// * `struct_builder` - A reference to the StructDefinitionBuilder
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn new_encapsulated(program: &'a Program, strict: bool, struct_builder: &'a StructDefinitionBuilder<'a>) -> Self {
        Self {
            program,
            opt_struct_builder: Some(struct_builder),
            strict,
            name: None,
            has_default_case: false,
            renames: HashMap::new(),
            annotations: vec![],
            doxygen_preceding_comment: vec![],
            doxygen_trailing_comment: vec![],
            cases: vec![]
        }
    }

    /// Create a new UnionDefinitionBuilder
    ///
    /// # Arguments
    ///
    /// * `def_name` - A string slice
    ///
    /// # Returns
    ///
    /// * True if the union has a forward declaration, else false
    fn check_has_forward_declaration(&self, def_name: &str) -> bool {
        let has_type = self.program.types.iter().any(|t| {
            match t.as_ref() {
                Type::Union { name } => name.as_str() == def_name,
                _ => false
            }
        });
        let has_definition = self.program.union_definitions.iter().any(|d| d.name == def_name);
        has_type && !has_definition
    }

    /// Create a new UnionDefinitionBuilder
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
        if !self.check_has_forward_declaration(&name) && (self.opt_struct_builder.is_some_and(|b| !b.check_member_name_is_free(&name)) || !self.program.check_symbol_name_is_free(&name)) {
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

    /// Create a new UnionDefinitionBuilder
    ///
    /// # Arguments
    ///
    /// * `rule` - Pair<Rule>
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn with_annotations(mut self, rule: Pair<Rule>) -> Result<Self, Box<Error<Rule>>> {
        if self.opt_struct_builder.is_some() {
            self.annotations = parse_member_annotations(rule, &self.program.language, self.strict)?;
        } else {
            let (ann, _) = parse_union_annotations(rule, &self.program.language, self.strict)?;
            self.annotations = ann;
        }
        Ok(self)
    }

    /// Create a new UnionDefinitionBuilder
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

    /// Create a new UnionDefinitionBuilder
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

    /// Create a new UnionDefinitionBuilder
    ///
    /// # Arguments
    ///
    /// * `case` - A UnionCase
    pub(crate) fn with_case(&mut self, case: UnionCase) {
        if case.case_values.is_empty() { // Default case
            self.has_default_case = true;
        }
        for member in case.members.iter() {
            self.check_name_annotation(&member.name, &member.annotations);
        }
        self.cases.push(case);
    }

    /// Check if the union case member name is free
    ///
    /// # Arguments
    ///
    /// * `name` - A string slice
    ///
    /// # Returns
    ///
    /// * True if the union case member name is free, else false
    pub(super) fn check_union_case_member_name_is_free(&self, name: &str) -> bool {
        self.cases.iter().all(|c| c.members.iter().all(|m| m.name != name))
    }


    /// Check if the union has a default case
    ///
    /// # Returns
    ///
    /// * True if the union has a default case, else false
    pub(super) fn check_has_default_case(&self) -> bool {
        self.cases.iter().any(|c| c.case_values.is_empty())
    }


    /// Create a new UnionDefinitionBuilder
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn new_case_builder(&'a self) -> UnionCaseBuilder<'a> {
        UnionCaseBuilder::new(self, self.strict)
    }

    /// Adds a rename mapping to the program.
    ///
    /// This function inserts a new rename mapping into the `renames` HashMap of the program.
    /// The `renames` HashMap is used to map from one name to another.
    ///
    /// # Arguments
    ///
    /// * `from` - A string slice that holds the old name.
    /// * `annotations` - A vector of annotations.
    pub(crate) fn check_name_annotation(&mut self, from: &str, annotations: &Vec<Annotation>) {
        if let Some(Annotation::Name {name, ..}) = find_name_annotation(annotations) {
            self.renames.insert(from.to_string(), name.clone());
        }
    }

    /// Build the UnionDefinition
    ///
    /// # Returns
    ///
    /// * The UnionDefinition
    pub(crate) fn build(self) -> UnionDefinition {
        UnionDefinition {
            name: self.name.unwrap(),
            annotations: self.annotations,
            renames: self.renames,
            doxygen_preceding_comment: self.doxygen_preceding_comment,
            doxygen_trailing_comment: self.doxygen_trailing_comment,
            cases: self.cases,
        }
    }


    /// Build the UnionDefinition
    ///
    /// # Arguments
    ///
    /// * `discriminator` - A Value
    ///
    /// # Returns
    ///
    /// * The StructMember
    pub(crate) fn build_encapsulated(self, discriminator: Value) -> StructMember {
        StructMember::UnionDefinition {
            definition: UnionDefinition {
                name: self.name.unwrap(),
                annotations: self.annotations,
                renames: self.renames,
                doxygen_preceding_comment: self.doxygen_preceding_comment,
                doxygen_trailing_comment: self.doxygen_trailing_comment,
                cases: self.cases,
            },
            discriminator 
        }
    }
}