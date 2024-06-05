/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use std::collections::{HashMap, HashSet};
use std::rc::Rc;
use pest::error::Error;
use pest::iterators::Pair;
use crate::parser::grammar_parser::Rule;
use crate::parser::util::annotations::{find_name_annotation, parse_interface_annotations};
use crate::symbols::annotations::Annotation;
use crate::symbols::doxygen_comment::DoxygenComment;
use crate::symbols::function_definition::{FunctionDefinition, FunctionDefinitionBuilder};
use crate::symbols::program::Program;
use crate::symbols::types::Type;

/// Interface definition
#[derive(Debug, Clone)]
pub(crate) struct InterfaceDefinition {
    pub(crate) name: String,
    pub(crate) id: Option<usize>,
    pub(crate) annotations: Vec<Annotation>,
    pub(crate) renames: HashMap<String, String>,
    pub(crate) callback_definitions: Vec<FunctionDefinition>,
    pub(crate) functions: Vec<FunctionDefinition>, 
    pub(crate) doxygen_preceding_comment: Vec<DoxygenComment>,
    pub(crate) doxygen_trailing_comment: Vec<DoxygenComment>,
}

impl InterfaceDefinition {
    pub fn apply_rename(&mut self, renames: &HashMap<String, String>) {
        self.name = renames.get(&self.name).unwrap_or(&self.name).to_string();
        self.callback_definitions.iter_mut().for_each(|f| f.apply_rename(&self.renames, renames));
        self.functions.iter_mut().for_each(|f| f.apply_rename(&self.renames, renames));
    }
}

/// Implementation of InterfaceDefinition
pub(crate) struct InterfaceDefinitionBuilder<'a> {
    pub(super) program: &'a Program,
    strict: bool,
    pub(crate) name: Option<String>,
    pub(crate) id: Option<usize>,
    pub(crate) function_ids: HashSet<usize>,
    annotations: Vec<Annotation>,
    renames: HashMap<String, String>,
    callback_definitions: Vec<FunctionDefinition>,
    functions: Vec<FunctionDefinition>,
    doxygen_preceding_comment: Vec<DoxygenComment>,
    doxygen_trailing_comment: Vec<DoxygenComment>,
}

/// Implementation of InterfaceDefinitionBuilder
impl<'a> InterfaceDefinitionBuilder<'a> {
    pub fn new(program: &'a Program, strict: bool) -> Self {
        Self {
            program,
            strict,
            name: None,
            id: None,
            function_ids: HashSet::new(),
            renames: HashMap::new(),
            annotations: vec![],
            callback_definitions: vec![],
            functions: vec![],
            doxygen_preceding_comment: vec![],
            doxygen_trailing_comment: vec![],
        }
    }

    /// Create a new InterfaceDefinitionBuilder
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


    /// Create a new InterfaceDefinitionBuilder
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

    /// Create a new InterfaceDefinitionBuilder
    ///
    /// # Arguments
    ///
    /// * `rule` - A Pair<Rule>
    /// * `scope` - An Option<String>
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn resolve_scoped_type(&self, rule: Pair<Rule>, scope: &Option<String>) -> Result<Rc<Type>, Box<Error<Rule>>> {
        let is_ident = rule.clone().into_inner().next().is_some_and(|r| r.as_rule() == Rule::ident);
        let name = rule.as_str().to_string();
        if is_ident && scope.as_ref().is_none() {
            let resolved = self.callback_definitions.iter().find(|f| f.name == name);
            if let Some(f) = resolved {
                return Ok(Rc::new(Type::Callback{ name: f.name.clone(), scope: self.name.clone().expect("Interface name must be set") }));
            }
        }
        if let Some(s) = scope { 
            return self.program.resolve_scoped_type(name.as_str(), s.as_str())
                .ok_or(Box::new(Error::new_from_span(
                    pest::error::ErrorVariant::CustomError {
                        message: format!("Type {} not found", name).to_string(),
                    },
                    rule.as_span(),
                )))
        }
        self.program.resolve_simple_data_type(rule)

    }

    /// Create a new InterfaceDefinitionBuilder
    ///
    /// # Arguments
    ///
    /// * `rule` - A Pair<Rule>
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn with_name(mut self, rule: Pair<Rule>) -> Result<Self, Box<Error<Rule>>> {
        let location = rule.as_span();
        let name = rule.as_str().to_string();
        // Either the name is not in use or it is used in a forward declaration
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

    /// Create a new InterfaceDefinitionBuilder
    ///
    /// # Arguments
    ///
    /// * `name` - A &str
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn check_function_name_is_free(&self, name: &str) -> bool {
        self.callback_definitions.iter().all(|f| f.name != name)
            && self.functions.iter().all(|f|  f.name != name
        ) && self.program.check_symbol_name_is_free(name) // Symbol name may not be necessary
    }

    /// Create a new InterfaceDefinitionBuilder
    ///
    /// # Arguments
    ///
    /// * `id` - A usize
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn check_function_id_is_free(&self, id: usize) -> bool {
        !self.function_ids.contains(&id)
    }

    /// Create a new InterfaceDefinitionBuilder
    ///
    /// # Arguments
    ///
    /// * `callback_definition` - A FunctionDefinition
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn add_callback_definition(&mut self, callback_definition: FunctionDefinition) {
        self.check_name_annotation(&callback_definition.name, &callback_definition.annotations);
        self.callback_definitions.push(callback_definition);
    }

    /// Create a new InterfaceDefinitionBuilder
    ///
    /// # Arguments
    ///
    /// * `function` - A FunctionDefinition
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn add_function(&mut self, function: FunctionDefinition) {
        if let Some(id) = function.id { 
            self.function_ids.insert(id);
        }
        self.check_name_annotation(&function.name, &function.annotations);
        self.functions.push(function);
    }


    /// Create a new InterfaceDefinitionBuilder
    ///
    /// # Arguments
    ///
    /// * `rule` - A Pair<Rule>
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn with_annotations(mut self, rule: Pair<Rule>) -> Result<Self, Box<Error<Rule>>> {
        self.annotations = parse_interface_annotations(self.program, rule, &self.program.language, self.strict)?;
        if let Some(Annotation::Id {id, .. }) = self.annotations.iter().find(|a| matches!(a, Annotation::Id {..})) {
            self.id = Some(*id);
        }
        Ok(self)
    }

    /// Create a new InterfaceDefinitionBuilder
    ///
    /// # Arguments
    ///
    /// * `name` - A Rc<Type>
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn get_callback_definition(&self, name: Rc<Type>) -> Option<FunctionDefinition> {
        let Type::Callback { name, .. } = name.as_ref() else { 
            return None;
        };
        self.callback_definitions.iter().find(|f| &f.name == name).cloned()
    }

    /// Create a new InterfaceDefinitionBuilder
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn new_function_definition_builder(&self) -> FunctionDefinitionBuilder {
        FunctionDefinitionBuilder::new(self, self.strict)
    }

    /// Create a new InterfaceDefinitionBuilder
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn assign_function_ids(&mut self) {
        let mut id = 1;
        for function in self.functions.iter_mut() {
            if function.id.is_none() {
                while self.function_ids.contains(&id) {
                    id += 1;
                }
                function.id = Some(id);
                id += 1;
            }
        }
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

    /// Create a new InterfaceDefinitionBuilder
    ///
    /// # Returns
    ///
    /// * The InterfaceDefinition
    pub(crate) fn build(mut self) -> InterfaceDefinition {
        self.assign_function_ids();
        InterfaceDefinition {
            name: self.name.expect("Interface name not set"),
            id: self.id,
            annotations: self.annotations,
            renames: self.renames,
            callback_definitions: self.callback_definitions,
            functions: self.functions,
            doxygen_preceding_comment: self.doxygen_preceding_comment,
            doxygen_trailing_comment: self.doxygen_trailing_comment,
        }
    }
}
