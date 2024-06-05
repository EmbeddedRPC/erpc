/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use std::collections::HashMap;
use crate::parser::grammar_parser::Rule;
use crate::parser::util::annotations::{find_name_annotation, parse_function_annotations};
use crate::parser::util::common::extract_simple_data_type;
use crate::parser::util::interface_definition::{
    extract_callback_parameters, extract_scope_ident, extract_void_rule,
};
use crate::symbols::annotations::Annotation;
use crate::symbols::doxygen_comment::DoxygenComment;
use crate::symbols::function_parameter::{FunctionParameter, FunctionParameterBuilder};
use crate::symbols::interface_definition::InterfaceDefinitionBuilder;
use crate::symbols::pending_ident::{PendingError, PendingIdent};
use crate::symbols::types::Type;
use log::info;
use pest::error::{Error, ErrorVariant};
use pest::iterators::Pair;
use pest::Position;
use std::hash::{Hash, Hasher};
use std::rc::Rc;

/// A function definition
#[derive(Debug, Clone)]
pub(crate) struct FunctionDefinition {
    pub(crate) name: String,
    pub(crate) id: Option<usize>,
    pub(crate) prototype: Option<Rc<Type>>,
    pub(crate) renames: HashMap<String, String>,
    pub(crate) is_oneway: bool,
    pub(crate) return_type: Option<Rc<Type>>,
    pub(crate) parameters: Vec<FunctionParameter>,
    pub(crate) annotations: Vec<Annotation>,
    pub(crate) doxygen_preceding_comment: Vec<DoxygenComment>,
    pub(crate) doxygen_trailing_comment: Vec<DoxygenComment>,
}

/// Implementation of the equality for the function definition
impl Hash for FunctionDefinition {
    fn hash<H: Hasher>(&self, state: &mut H) {
        self.name.hash(state);
    }
}

impl FunctionDefinition {
    pub fn apply_rename(&mut self, interface_renames: &HashMap<String, String>, program_renames: &HashMap<String, String>) {
        self.name = interface_renames.get(&self.name).unwrap_or(&self.name).to_string();
        self.parameters.iter_mut().for_each(|m| m.apply_rename(&self.renames, program_renames));
    }
}

/// A builder for a function definition
pub(crate) struct FunctionDefinitionBuilder<'a> {
    pub(crate) interface_builder: &'a InterfaceDefinitionBuilder<'a>,
    strict: bool,
    name: Option<String>,
    id: Option<usize>,
    prototype: Option<Rc<Type>>,
    renames: HashMap<String, String>,
    is_oneway: bool,
    return_type: Option<Rc<Type>>,
    parameters: Vec<FunctionParameter>,
    annotations: Vec<Annotation>,
    doxygen_preceding_comment: Vec<DoxygenComment>,
    doxygen_trailing_comment: Vec<DoxygenComment>,
    pending_idents: Vec<PendingIdent>,
}

/// Implementation of the builder
impl<'a> FunctionDefinitionBuilder<'a> {
    pub fn new(interface_builder: &'a InterfaceDefinitionBuilder<'a>, strict: bool) -> Self {
        Self {
            interface_builder,
            strict,
            name: None,
            id: None,
            prototype: None,
            renames: HashMap::new(),
            is_oneway: false,
            return_type: None,
            parameters: vec![],
            annotations: vec![],
            doxygen_preceding_comment: vec![],
            doxygen_trailing_comment: vec![],
            pending_idents: vec![],
        }
    }

    /// Creates a new function definition builder from a prototype
    ///
    /// # Arguments
    ///
    /// * `rule` - The rule containing the prototype
    ///
    /// # Returns
    ///
    /// * The function definition builder
    pub(crate) fn from_prototype(mut self, rule: Pair<Rule>) -> Result<Self, Box<Error<Rule>>> {
        let rules = rule.into_inner();
        let scope_ident = extract_scope_ident(&rules).map(|ident| ident.as_str().to_string());
        let simple_data_type =
            extract_simple_data_type(&rules).expect("Simple data type is required");
        let res = self
            .interface_builder
            .resolve_scoped_type(simple_data_type, &scope_ident)?;
        self.prototype = Some(res.clone());
        self.copy_prototype(res);
        Ok(self)
    }

    /// Copies the prototype of the function
    ///
    /// # Arguments
    ///
    /// * `prototype` - The prototype to copy
    fn copy_prototype(&mut self, prototype: Rc<Type>) {
        if let Some(prototype) = self
            .interface_builder
            .get_callback_definition(prototype.clone())
        {
            self.copy_prototype_properties(prototype);
            return;
        }
        if let Some(prototype) = self
            .interface_builder
            .program
            .resolve_function_definition(prototype.clone())
        {
            self.copy_prototype_properties(prototype);
        }
    }

    /// Copies the properties of the prototype
    ///
    /// # Arguments
    ///
    /// * `prototype` - The prototype to copy
    pub(crate) fn copy_prototype_properties(&mut self, prototype: FunctionDefinition) {
        self.is_oneway = prototype.is_oneway;
        self.return_type = prototype.return_type;
        self.parameters = prototype.parameters;
    }

    /// Adds a parameter to the function definition
    ///
    /// This method adds a parameter to the function definition.
    /// It first checks if the parameter name is free, then it checks if the parameter type is valid.
    /// If the parameter is valid, it adds it to the function definition.
    ///
    /// # Arguments
    ///
    /// * `rule` - The rule containing the parameter
    ///
    /// # Returns
    ///
    /// * The builder
    ///
    /// # Errors
    ///
    /// * Too much parameters for function
    /// * Expected more parameter names
    /// * Parameter must exist

    pub(crate) fn with_renamed_parameters(
        mut self,
        rule: Option<Pair<Rule>>,
    ) -> Result<Self, Box<Error<Rule>>> {
        let Some(rule) = rule else {
            return Ok(self);
        };
        let rules = rule.clone().into_inner();
        let idents = extract_callback_parameters(rules); // This statement is true, because it is handled by with_type
        for (idx, ident) in idents.iter().enumerate() {
            if idx > self.parameters.len() {
                return Err(Box::new(Error::new_from_span(
                    ErrorVariant::CustomError {
                        message: "Too much parameters for function".to_string(),
                    },
                    ident.as_span(),
                )));
            }
            let parameter = self.parameters.get_mut(idx).expect("Parameter must exist");
            parameter.name = Some(ident.as_str().to_string());
        }
        let all_named = self.check_all_parameters_have_names();
        if !all_named {
            return Err(Box::new(Error::new_from_pos(
                ErrorVariant::CustomError {
                    message: "Expected more parameter names".to_string(),
                },
                rule.as_span().end_pos(),
            )));
        }

        Ok(self)
    }

    /// Adds a parameter to the function definition
    ///
    /// # Arguments
    ///
    /// * `comment` - The comment to add
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn with_doxygen_preceding_comment(mut self, comment: Vec<DoxygenComment>) -> Self {
        self.doxygen_preceding_comment = comment;
        self
    }

    /// Adds a parameter to the function definition
    ///
    /// # Arguments
    ///
    /// * `comment` - The comment to add
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn with_doxygen_trailing_comment(mut self, comment: Vec<DoxygenComment>) -> Self {
        self.doxygen_trailing_comment = comment;
        self
    }

    /// Adds a parameter to the function definition
    ///
    /// # Arguments
    ///
    /// * `is_oneway` - The flag to set
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn with_oneway(mut self, is_oneway: bool) -> Self {
        self.is_oneway = is_oneway;
        self
    }

    /// Adds a parameter to the function definition
    ///
    /// # Arguments
    ///
    /// * `rule` - The rule containing the name
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn with_name(mut self, rule: Pair<Rule>) -> Result<Self, Box<Error<Rule>>> {
        let location = rule.as_span();
        let name = rule.as_str().to_string();
        if !self.interface_builder.check_function_name_is_free(&name) {
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

    /// Adds a parameter to the function definition
    ///
    /// # Arguments
    ///
    /// * `rule` - The rule containing the name
    /// * `unnamed` - The flag to set
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn with_name_cb(
        mut self,
        rule: Pair<Rule>,
        unnamed: bool,
    ) -> Result<Self, Box<Error<Rule>>> {
        let location = rule.as_span();
        let name = rule.as_str().to_string();
        if !self.interface_builder.check_function_name_is_free(&name) {
            return Err(Box::new(Error::new_from_span(
                ErrorVariant::CustomError {
                    message: format!("Symbol name {} is already in use", name).to_string(),
                },
                location,
            )));
        }
        if unnamed {
            let ok = self.check_all_parameters_have_names();
            if !ok {
                return Err(Box::new(Error::new_from_pos(
                    ErrorVariant::CustomError {
                        message: "Expected parameter name".to_string(),
                    },
                    location.end_pos(),
                )));
            }
        }

        self.name = Some(name);
        Ok(self)
    }

    /// Checks if all parameters of the function have names.
    ///
    /// This method iterates over all parameters of the function and checks if they have a name.
    /// It returns `true` if all parameters have a name, and `false` otherwise.
    ///
    /// # Returns
    ///
    /// * `bool` - `true` if all parameters have names, `false` otherwise.
    fn check_all_parameters_have_names(&self) -> bool {
        self.parameters.iter().all(|p| p.name.is_some())
    }

    /// Adds a parameter to the function definition
    ///
    /// # Arguments
    ///
    /// * `type_rule` - The rule containing the type
    /// * `pos` - The position of the rule
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn with_return_type(
        mut self,
        type_rule: Option<Pair<Rule>>,
        pos: Position,
    ) -> Result<Self, Box<Error<Rule>>> {
        let Some(pair) = type_rule else {
            return if self.is_oneway {
                Ok(self)
            } else {
                Err(Box::new(Error::new_from_pos(
                    ErrorVariant::CustomError {
                        message: "Return type is required".to_string(),
                    },
                    pos,
                )))
            }
        };
        if self.is_oneway {
            return Err(Box::new(Error::new_from_span(
                ErrorVariant::CustomError {
                    message: "Oneway functions cannot have a return type".to_string(),
                },
                pair.as_span(),
            )));
        }
        let rules = pair.clone().into_inner();

        if extract_void_rule(&rules).is_some() {
            self.is_oneway = false;
            return Ok(self);
        };

        let Some(sdt) = extract_simple_data_type(&rules) else {
            return Err(Box::new(Error::new_from_span(
                ErrorVariant::CustomError {
                    message: "Invalid return type".to_string(),
                },
                pair.as_span(),
            )));
        };

        let return_type = self
            .interface_builder
            .program
            .resolve_simple_data_type(sdt)?;

        self.return_type = Some(return_type);
        Ok(self)
    }

    /// Checks if a parameter name is free
    ///
    /// # Arguments
    ///
    /// * `name` - The name to check
    ///
    /// # Returns
    ///
    /// * `bool` - `true` if the name is free, `false` otherwise
    pub(crate) fn check_parameter_name_is_free(&self, name: &str) -> bool {
        self.parameters
            .iter()
            .filter(|p| p.name.is_some())
            .all(|p| p.name.as_ref().unwrap() != name)
    }

    /// Creates a new parameter builder
    ///
    /// # Returns
    ///
    /// * The parameter builder
    pub(crate) fn new_parameter_builder(&self) -> FunctionParameterBuilder {
        FunctionParameterBuilder::new(self, self.strict)
    }

    /// Adds a parameter to the function definition
    ///
    /// # Arguments
    ///
    /// * `parameter` - The parameter to add
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn add_parameter(
        &mut self,
        parameter: FunctionParameter,
        pending_idents: Vec<PendingIdent>,
    ) {
        info!("Adding parameter {:?}", parameter);
        self.check_name_annotation(parameter.name.as_ref().unwrap(), &parameter.annotations);
        self.parameters.push(parameter);
        self.pending_idents.extend(pending_idents);
    }

    /// Adds annotations to the function definition
    ///
    /// # Arguments
    ///
    /// * `annotation_rule` - The rule containing the annotations
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn with_annotations(
        mut self,
        annotation_rule: Pair<Rule>,
    ) -> Result<Self, Box<Error<Rule>>> {
        let annotations = parse_function_annotations(
            self.interface_builder.program,
            self.interface_builder,
            annotation_rule,
            &self.interface_builder.program.language,
            self.strict,
        )?;
        if let Some(Annotation::Id { id, .. }) = annotations
            .iter()
            .find(|a| matches!(a, Annotation::Id { .. }))
        {
            self.id = Some(*id);
        }
        self.annotations = annotations;
        Ok(self)
    }

    /// Resolves pending identifiers
    ///
    /// # Returns
    ///
    /// * The result of the resolution
    ///
    /// # Errors
    ///
    /// * Symbol name is not a member of the function
    pub(crate) fn resolve_pending_idents(&mut self) -> Result<(), PendingError> {
        for pending in &self.pending_idents {
            let member = self
                .parameters
                .iter()
                .find(|m| m.name.clone().is_some_and(|n| n == pending.name));
            if let Some(m) = member {
                if let Some(e) = (pending.check)(m.type_.clone().as_ref()) {
                    return Err(PendingError::new(pending.name.clone(), pending.position, e));
                }
            } else {
                return Err(PendingError::new(
                    pending.name.clone(),
                    pending.position,
                    format!(
                        "Symbol name {} is not a member of the function",
                        pending.name
                    ),
                ));
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

    /// Builds the function definition
    ///
    /// # Returns
    ///
    /// * The function definition
    pub(crate) fn build(self) -> FunctionDefinition {
        FunctionDefinition {
            name: self.name.expect("Function name not set"),
            id: self.id,
            prototype: self.prototype,
            renames: self.renames,
            is_oneway: self.is_oneway,
            return_type: self.return_type,
            parameters: self.parameters,
            annotations: self.annotations,
            doxygen_preceding_comment: self.doxygen_preceding_comment,
            doxygen_trailing_comment: self.doxygen_trailing_comment,
        }
    }
}
