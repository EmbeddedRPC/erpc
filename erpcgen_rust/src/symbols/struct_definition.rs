/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use std::collections::HashMap;
use std::hash::{Hash, Hasher};
use pest::error::Error;
use pest::iterators::Pair;
use crate::parser::grammar_parser::Rule;
use crate::parser::util::annotations::{find_name_annotation, parse_struct_annotations};
use crate::symbols::annotations::Annotation;
use crate::symbols::doxygen_comment::DoxygenComment;
use crate::symbols::pending_ident::{CHECK_SCALAR, PendingError, PendingIdent};
use crate::symbols::program::Program;
use crate::symbols::struct_data_member::{StructDataMemberBuilder};
use crate::symbols::struct_member::StructMember;
use crate::symbols::types::Type;
use crate::symbols::union_definition::UnionDefinitionBuilder;
use crate::symbols::value::Value;

/// Struct definition
#[derive(Debug, PartialEq, Clone)]
pub(crate) struct StructDefinition {
    pub(crate) name: String,
    pub(crate) annotations: Vec<Annotation>,
    pub(crate) renames: HashMap<String, String>,
    pub(crate) members: Vec<StructMember>,
    pub(crate) doxygen_preceding_comment: Vec<DoxygenComment>,
    pub(crate) doxygen_trailing_comment: Vec<DoxygenComment>,
}

/// Hash implementation for StructDefinition
impl Hash for StructDefinition {
    fn hash<H: Hasher>(&self, state: &mut H) {
        self.name.hash(state);
    }
}

impl StructDefinition {
    pub fn apply_rename(&mut self, renames: &HashMap<String, String>) {
        self.name = renames.get(&self.name).unwrap_or(&self.name).to_string();
        self.members.iter_mut().for_each(|m| m.apply_rename(&self.renames, renames));
    }
}

/// Equality implementation for StructDefinition
impl Eq for StructDefinition {}

/// Struct definition builder
pub(crate) struct StructDefinitionBuilder<'a> {
    pub(super) program: &'a Program,
    strict: bool,
    name: Option<String>,
    renames: HashMap<String, String>,
    annotations: Vec<Annotation>,
    members: Vec<StructMember>,
    doxygen_preceding_comment: Vec<DoxygenComment>,
    doxygen_trailing_comment: Vec<DoxygenComment>,
    pending_idents: Vec<PendingIdent>
}


/// Implementation of StructDefinitionBuilder
impl<'a> StructDefinitionBuilder<'a> {
    pub fn new(program: &'a Program, strict: bool) -> Self {
        Self {
            program,
            strict,
            name: None,
            annotations: vec![],
            renames: HashMap::new(),
            members: vec![],
            doxygen_preceding_comment: vec![],
            doxygen_trailing_comment: vec![],
            pending_idents: vec![]
        }
    }

    /// Create a new StructDefinitionBuilder
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

    /// Create a new StructDefinitionBuilder
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

    /// Check if the struct has a forward declaration
    ///
    /// # Arguments
    ///
    /// * `def_name` - The name of the struct
    ///
    /// # Returns
    ///
    /// * True if the struct has a forward declaration, else false
    fn check_has_forward_declaration(&self, def_name: &str) -> bool {
        let has_type = self.program.types.iter().any(|t| { 
            match t.as_ref() {
                Type::Struct { name } => name.as_str() == def_name,
                _ => false
            }
        });
        let has_definition = self.program.struct_definitions.iter().any(|d| d.name == def_name);
        has_type && !has_definition
    }

    /// Create a new StructDefinitionBuilder
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
        if self.name.is_some() { 
            return Ok(self)
        }
        let location = rule.as_span();
        let name = rule.as_str().to_string();
        // Either the name is not in use or it is used in a forward declaration
        if !self.check_has_forward_declaration(&name) && !self.program.check_symbol_name_is_free(&name) {
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

    /// Create a new StructDefinitionBuilder
    ///
    /// # Arguments
    ///
    /// * `rule` - Pair<Rule>
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn with_annotations(mut self, rule: Pair<Rule>) -> Result<Self, Box<Error<Rule>>> {
        self.annotations = parse_struct_annotations(rule, &self.program.language, self.strict)?;
        Ok(self)
    }

    /// Create a new StructDefinitionBuilder
    ///
    /// # Arguments
    ///
    /// * `name` - A &str
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn check_member_name_is_free(&self, name: &str) -> bool {
        self.members.iter().all(|member|
            match member {
                StructMember::DataMember(f) => f.name != name,
                StructMember::UnionDefinition{ definition: d,.. } => d.name != name,
            }
        )
    }

    /// Add a new member
    ///
    /// # Arguments
    ///
    /// * `member` - StructMember
    /// * `pending_idents` - A vector of PendingIdent
    pub(crate) fn add_member(&mut self, member: StructMember, pending_idents: Vec<PendingIdent>) {
        self.pending_idents.extend(pending_idents);
        self.check_member_name_annotations(&member);
        self.members.push(member);
    }

    /// Create a new StructDataMemberBuilder
    pub(crate) fn new_member_builder(&self, strict: bool) -> StructDataMemberBuilder {
        StructDataMemberBuilder::new(self, strict)
    }

    /// Create a new UnionDefinitionBuilder
    pub(crate) fn new_union_builder(&self, strict: bool) -> UnionDefinitionBuilder {
        UnionDefinitionBuilder::new_encapsulated(self.program, strict, self)
    }

    /// Get a discriminator by identifier
    ///
    /// # Arguments
    ///
    /// * `pair` - Pair<Rule>
    ///
    /// # Returns
    ///
    /// * The value and a vector of PendingIdent
    pub(crate) fn get_discriminator_by_ident(&self, pair: Pair<Rule>) -> Result<(Value, Vec<PendingIdent>), Box<Error<Rule>>> {
        let ident = pair.as_str().to_string();
        Ok((Value::RuntimeValue { name: ident.clone() }, vec![PendingIdent::new(ident, pair.as_span().start_pos().pos(), Box::new(CHECK_SCALAR))]))
    }

    /// Resolve pending identifiers
    ///
    /// # Returns
    ///
    /// * Ok if successful, else an error
    pub(crate) fn resolve_pending_idents(&mut self) -> Result<(), PendingError> {
        for pending in &self.pending_idents {
            let member = self.members.iter().find(|m| match m {
                StructMember::DataMember(f) => f.name == pending.name,
                StructMember::UnionDefinition { .. } => false
            });
            if let Some(StructMember::DataMember(m)) = member {
                if let Some(e) =  (pending.check)(m.member_type.clone().as_ref()) {
                    return Err(PendingError::new(pending.name.clone(), pending.position, e))
                }
            } else  {
                return Err(PendingError::new(pending.name.clone(), pending.position, format!("Symbol name {} is not a member of the struct", pending.name)));
            }
        }
        Ok(())
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
    
    fn check_member_name_annotations(&mut self, member: &StructMember) {
        match member {
            StructMember::DataMember(f) => self.check_name_annotation(&f.name, &f.annotations),
            StructMember::UnionDefinition{ definition: d, .. } => self.check_name_annotation(&d.name, &d.annotations),
        }
    }

    /// Build the StructDefinition
    ///
    /// # Returns
    ///
    /// * The struct definitions.
    pub(crate) fn build(self) -> Result<StructDefinition, Box<Error<Rule>>> {
        Ok(StructDefinition {
            name: self.name.unwrap(),
            annotations: self.annotations,
            renames: self.renames,
            members: self.members,
            doxygen_preceding_comment: self.doxygen_preceding_comment,
            doxygen_trailing_comment: self.doxygen_trailing_comment,
        })
    }
}