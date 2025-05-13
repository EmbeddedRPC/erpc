/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use std::collections::HashMap;
use std::rc::Rc;
use log::trace;
use pest::error::{Error, ErrorVariant};
use pest::iterators::Pair;
use crate::parser::grammar_parser::Rule;
use crate::symbols::pending_ident::{PendingError, PendingIdent};
use crate::symbols::struct_member::StructMember;
use crate::symbols::types::Type;
use crate::symbols::union_case_member::{UnionCaseMember, UnionCaseMemberBuilder};
use crate::symbols::union_definition::UnionDefinitionBuilder;
use crate::symbols::value::Value;

/// Union case
#[derive(Debug, PartialEq, Clone)]
pub(crate) struct UnionCase {
    pub(crate) case_type: Rc<Type>,
    pub(crate) case_values: Vec<Value>, // Empty signifies default case
    pub(crate) members: Vec<UnionCaseMember>,
}

/// Implementation of UnionCase
impl UnionCase {
    /// Apply rename to the UnionCase
    ///
    /// # Arguments
    ///
    /// * `renames` - HashMap<String, String>
    /// * `program_renames` - HashMap<String, String>
    pub(crate) fn apply_rename(&mut self, union_renames: &HashMap<String, String>, program_renames: &HashMap<String, String>) {
        self.case_values = self.case_values.iter().map(|v| 
            if let Value::ReferencedValue { name, prefix, value } = v {
                let new_name = program_renames.get(name).unwrap_or(name).to_string();
                let new_prefix = prefix.clone().map(|p| program_renames.get(&p).unwrap_or(&p).to_string());
                Value::ReferencedValue { name: new_name, value: value.clone(), prefix: new_prefix }
            } else {
                v.clone()
            }
        ).collect();
        self.members.iter_mut().for_each(|m| m.apply_rename(union_renames, program_renames));
    }
}

/// Union case builder
pub(crate) struct UnionCaseBuilder<'a> {
    pub(super) union_builder: &'a UnionDefinitionBuilder<'a>,
    strict: bool,
    case_type: Option<Rc<Type>>,
    case_values: Vec<Value>,
    members: Vec<UnionCaseMember>,
    pending_idents: Vec<PendingIdent>
}

/// Implementation of UnionCaseBuilder
impl<'a> UnionCaseBuilder<'a> {
    pub(super) fn new(union_builder: &'a UnionDefinitionBuilder<'a>, strict: bool) -> UnionCaseBuilder<'a> {
        Self {
            union_builder,
            strict,
            case_type: Some(Rc::new(Type::Int32)),
            case_values: vec![],
            members: vec![],
            pending_idents: vec![]
        }
    }

    /// Create a new UnionCaseBuilder
    ///
    /// # Arguments
    ///
    /// * `union_case_values_rule` - Option<Pair<Rule>>
    ///
    /// # Returns
    ///
    /// * The builder.
    pub(crate) fn with_case_values(mut self, union_case_values_rule: Option<Pair<Rule>>) -> Result<Self, Box<Error<Rule>>> {
        let Some(union_case_values_rule) = union_case_values_rule else {
            return Ok(self)
        };
        let mut case_values = Vec::new();
        trace!("With case values {:?}", union_case_values_rule.as_str());
        let value_rules = union_case_values_rule.into_inner();
        for value_rule in value_rules.find_tagged("exp") {
            let value = self.evaluate_case_value(value_rule)?;
            case_values.push(value);
        }
        self.case_values = case_values;
        Ok(self)
    }

    /// Evaluate case value
    ///
    /// # Arguments
    ///
    /// * `expression_rule` - Pair<Rule>
    ///
    /// # Returns
    ///
    /// * The value or error
    ///
    /// # Errors
    ///
    /// * If the case value is not an integer
    fn evaluate_case_value(&self, expression_rule: Pair<Rule>) -> Result<Value, Box<Error<Rule>>> {
        let span = expression_rule.as_span();
        let value = self.union_builder.program.resolve_value_or_reference_from_expression(expression_rule, &Type::Int32, self.strict)?;

        if !value.is_integer() && !value.is_referenced() {
            return Err(Box::new(Error::new_from_span(
                ErrorVariant::CustomError {
                    message: "Case member value must be an integer".to_string(),
                },
                span,
            )));
        }
        Ok(value)
    }


    /// Create a new UnionCaseBuilder
    ///
    /// # Arguments
    ///
    /// * `member` - UnionCaseMember
    /// * `pending` - Vec<PendingIdent>
    pub(crate) fn with_member(&mut self, member: UnionCaseMember, pending: Vec<PendingIdent>) {
        self.pending_idents.extend(pending);
        self.members.push(member);
    }


    /// Create a new UnionCaseBuilder
    ///
    /// # Arguments
    ///
    /// * `default_rule` - Option<Pair<Rule>>
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn check_if_default_case(self, default_rule: Option<Pair<Rule>>) -> Result<Self, Box<Error<Rule>>> {
        if let Some(default_rule) = default_rule {
            if self.union_builder.has_default_case {
                return Err(Box::new(Error::new_from_span(
                    ErrorVariant::CustomError {
                        message: "Default case must not have case values".to_string(),
                    },
                    default_rule.as_span(),
                )));
            }
        }
        Ok(self)
    }

    /// Resolve pending identifiers
    ///
    /// # Returns
    ///
    /// * Ok if all pending identifiers are resolved, else an error
    pub(crate) fn resolve_pending_idents(&mut self) -> Result<(), PendingError> {
        for pending in &self.pending_idents {
            let member = self.members.iter().find(|m| m.name == pending.name);
            if let Some(m) = member {
                if let Some(e) =  (pending.check)(m.member_type.clone().as_ref()) {
                    return Err(PendingError::new(pending.name.clone(), pending.position, e))
                }
            } else  {
                return Err(PendingError::new(pending.name.clone(), pending.position, format!("Symbol name {} is not a member of the union", pending.name)));
            }
        }
        Ok(())
    }

    /// Create a new UnionCaseMemberBuilder
    ///
    /// # Returns
    ///
    /// * The builder
    pub(crate) fn new_member_builder(&'a self) -> UnionCaseMemberBuilder<'a> {
        UnionCaseMemberBuilder::new(self, self.strict)
    }

    /// Build the UnionCase
    ///
    /// # Returns
    ///
    /// * The UnionCase
    pub(crate) fn build(mut self) -> Result<UnionCase, Box<Error<Rule>>> {
        Ok(UnionCase {
            case_type: self.case_type.unwrap(),
            case_values: self.case_values,
            members: self.members,
        })
    }
}