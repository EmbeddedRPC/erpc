/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use std::collections::HashMap;
use std::rc::Rc;
use log::{debug, info};
use pest::error::{Error, ErrorVariant};
use pest::iterators::Pair;
use crate::parser::grammar_parser::Rule;
use crate::parser::util::annotations::{find_name_annotation, has_discriminator_annotation, parse_array_annotations, parse_enum_annotations, parse_list_annotations, parse_member_annotations, parse_struct_annotations, parse_union_annotations};
use crate::parser::util::common::{extract_simple_data_type};
use crate::parser::util::interface_definition::extract_scope_ident;
use crate::symbols::annotations::Annotation;
use crate::symbols::function_definition::FunctionDefinitionBuilder;
use crate::symbols::param_direction::ParamDirection;
use crate::symbols::pending_ident::PendingIdent;
use crate::symbols::types::Type;
use crate::symbols::util::{apply_rename_for_annotation, apply_rename_for_type};

/// A function parameter
#[derive(Debug, Clone)]
pub(crate) struct FunctionParameter {
    pub(crate) name: Option<String>,
    pub(crate) type_: Rc<Type>,
    pub(crate) direction: ParamDirection,
    pub(crate) annotations: Vec<Annotation>,
}

impl FunctionParameter {
    pub fn apply_rename(&mut self, renames: &HashMap<String, String>, program_renames: &HashMap<String, String>) {
        if let Some(name) = &self.name {
            self.name = Some(renames.get(name).unwrap_or(name).to_string());
        }
        self.annotations = self.annotations.iter().map(|a| apply_rename_for_annotation(a.clone(), renames, program_renames)).collect();
        self.type_ = apply_rename_for_type(self.type_.clone(), program_renames);
    }
}

/// A builder for a function parameter
pub(crate) struct FunctionParameterBuilder<'a> {
    function_builder: &'a FunctionDefinitionBuilder<'a>,
    strict: bool,
    name: Option<String>,
    type_: Option<Rc<Type>>,
    direction: ParamDirection,
    annotations: Vec<Annotation>,
    pending_idents: Vec<PendingIdent>
}

/// Implementation of the builder
impl<'a> FunctionParameterBuilder<'a> {
    pub(crate) fn new(function_builder: &'a FunctionDefinitionBuilder<'a>, strict: bool) -> Self {
        Self {
            function_builder,
            strict,
            name: None,
            type_: None,
            direction: ParamDirection::In,
            annotations: vec![],
            pending_idents: vec![]
        }
    }

    /// Set the name of the parameter
    ///
    /// # Arguments
    ///
    /// * `rule` - The rule to extract the name from
    ///
    /// # Returns
    ///
    /// * The builder
    ///
    /// # Errors
    ///
    /// * If the name is already in use
    ///
    pub(crate) fn with_name(mut self, rule: Option<Pair<Rule>>) -> Result<Self, Box<Error<Rule>>> {
        let Some(rule) = rule else { 
            return Ok(self)
        };
        if self.name.is_some() { 
            return Ok(self)
        }
        let location = rule.as_span();
        let name = rule.as_str().to_string();
        debug!("With name: {}", name);
        if !self.function_builder.check_parameter_name_is_free(&name) {
            return Err(Box::new(Error::new_from_span(
                ErrorVariant::CustomError {
                    message: format!("Symbol name {} is already in use", name.as_str())
                },
                location,
            )));
        }
        self.name = Some(name);
        Ok(self)
    }

    /// Set the type of the parameter
    ///
    /// # Arguments
    ///
    /// * `rule` - The rule to extract the type from
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn with_type(mut self, rule: Pair<Rule>) -> Result<Self, Box<Error<Rule>>> {
        let rules = rule.into_inner();
        let scope_ident = extract_scope_ident(&rules).map(|ident| ident.as_str().to_string());
        let simple_data_type = extract_simple_data_type(&rules).expect("Simple data type is required");
        let res = self.function_builder.interface_builder.resolve_scoped_type(simple_data_type, &scope_ident)?;
        debug!("With type: {:?}", res);
        self.type_ = Some(res);
        Ok(self)
    }

    /// Set the type of the parameter
    ///
    /// # Arguments
    ///
    /// * `annotations_rule` - The rule to extract the annotations from
    ///
    /// # Returns
    ///
    /// * The builder
    ///
    /// # Errors
    ///
    /// * Type must be defined
    /// * Union member must have a discriminator annotation
    pub(crate) fn with_annotations(mut self, annotations_rule: Pair<Rule>) -> Result<Self, Box<Error<Rule>>> {
        let cloned = annotations_rule.clone();
        self.annotations = match self.type_.clone().expect("Type must be defined").as_ref() {
            Type::Struct { .. } => parse_struct_annotations(annotations_rule, &self.function_builder.interface_builder.program.language, self.strict)?,
            Type::Enum { .. } => parse_enum_annotations(annotations_rule, &self.function_builder.interface_builder.program.language, self.strict)?,
            Type::Union { .. } => {
                let (ann, pend) = parse_union_annotations(annotations_rule,&self.function_builder.interface_builder.program.language , self.strict)?;
                if !has_discriminator_annotation(&ann) {
                    return Err(Box::new(Error::new_from_pos(
                        ErrorVariant::CustomError {
                            message: "Union member must have a discriminator annotation".to_string(),
                        },
                        cloned.as_span().start_pos(),
                    )));
                }
                self.pending_idents.extend(pend);
                ann
            },
            Type::Array { .. } => parse_array_annotations(annotations_rule, &self.function_builder.interface_builder.program.language, self.strict)?,
            Type::List { .. } => {
                let (ann, pend) = parse_list_annotations(self.function_builder.interface_builder.program, annotations_rule, &self.function_builder.interface_builder.program.language, self.strict)?;
                self.pending_idents.extend(pend);
                ann
            },
            Type::Binary => {
                let (ann, pend) = parse_list_annotations(self.function_builder.interface_builder.program, annotations_rule, &self.function_builder.interface_builder.program.language, self.strict)?;
                self.pending_idents.extend(pend);
                ann
            },
            _ => parse_member_annotations(annotations_rule.clone(), &self.function_builder.interface_builder.program.language, self.strict)?
        };
        if self.type_.clone().is_some_and(|t| t.is_union()) && !has_discriminator_annotation(&self.annotations) {
            return Err(Box::new(Error::new_from_pos(
                ErrorVariant::CustomError {
                    message: "Union member must have a discriminator annotation".to_string(),
                },
                cloned.as_span().start_pos(),
            )));
        }
        if let Some(Annotation::Name {name, ..}) = find_name_annotation(&self.annotations) {
            self.name = Some(name.clone()); // Consider adding a check for name conflict
        }
        Ok(self)
    }

    /// Set the direction of the parameter
    ///
    /// # Arguments
    ///
    /// * `direction` - The rule to extract the direction from
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn with_direction(mut self, direction: Pair<Rule>) -> Self {
        let direction = match direction.as_rule() {
            Rule::paramDirectionIn => ParamDirection::In,
            Rule::paramDirectionOut => ParamDirection::Out,
            Rule::paramDirectionInout => ParamDirection::InOut,
            _ => unreachable!()
        };
        debug!("With direction: {:?}", direction);
        self.direction = direction;
        self
    }

    /// Build the function parameter
    ///
    /// # Returns
    ///
    /// * The function parameter
    pub(crate) fn build(self) -> (FunctionParameter, Vec<PendingIdent>) {
        info!("Building function parameter {:?}", self.name);
        (FunctionParameter {
            name: self.name,
            type_: self.type_.expect("Type is required"),
            direction: self.direction,
            annotations: self.annotations
        }, self.pending_idents)
    }
}