/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use std::rc::Rc;

use crate::generator::python::util::{
    find_discriminator_name, find_external_annotation, format_comments,
    is_discriminator_for_function, is_length_for_function,
};
use crate::generator::util::find_nullable_annotation;
use crate::symbols::function_definition::FunctionDefinition;
use crate::symbols::function_parameter::FunctionParameter;
use crate::symbols::param_direction::ParamDirection;
use crate::symbols::types::Type;

pub struct PyFunction {
    pub name: String,
    pub id: usize,
    pub return_type: Option<Rc<Type>>,
    pub is_oneway: bool,
    pub external: bool,
    pub parameters: Vec<PyFunctionParameter>,
    pub preceding_comment: String,
    pub trailing_comment: String,
}

impl PyFunction {
    pub(crate) fn from(function: &FunctionDefinition) -> Self {
        Self {
            name: function.name.clone(),
            id: function.id.expect("Function ID must be set"),
            return_type: function.return_type.clone(),
            is_oneway: function.is_oneway,
            external: find_external_annotation(&function.annotations),
            parameters: function
                .parameters
                .iter()
                .map(|p| {
                    PyFunctionParameter::from(
                        p,
                        is_length_for_function(
                            function,
                            &p.clone().name.expect("Parameter must have a name"),
                        ),
                        is_discriminator_for_function(
                            function,
                            &p.clone().name.expect("Parameter must have a name"),
                        ),
                    )
                })
                .collect(),
            preceding_comment: format_comments(function.doxygen_preceding_comment.clone()),
            trailing_comment: format_comments(function.doxygen_trailing_comment.clone()),
        }
    }

    pub fn has_out_parameters(&self) -> bool {
        self.parameters.iter().any(|p| p.is_out())
    }
}

pub struct PyFunctionParameter {
    pub name: String,
    pub type_: Rc<Type>,
    pub direction: ParamDirection,
    pub is_length_for: Option<String>,
    pub is_discriminator: Option<String>,
    pub discriminator_name: Option<String>,
    pub is_nullable: bool,
}

impl PyFunctionParameter {
    pub(crate) fn from(
        parameter: &FunctionParameter,
        is_length_for: Option<String>,
        is_discriminator_for: Option<String>,
    ) -> Self {
        Self {
            name: parameter.name.clone().expect("Parameter name must be set"),
            type_: parameter.type_.clone().resolve_original_type(),
            direction: parameter.direction,
            is_length_for: is_length_for.clone(),
            is_discriminator: is_discriminator_for.clone(),
            discriminator_name: find_discriminator_name(&parameter.annotations),
            is_nullable: find_nullable_annotation(&parameter.annotations)
        }
    }

    pub(crate) fn is_out(&self) -> bool {
        self.direction == ParamDirection::Out || self.direction == ParamDirection::InOut
    }

    pub(crate) fn is_in(&self) -> bool {
        self.direction == ParamDirection::In || self.direction == ParamDirection::InOut
    }

    pub(crate) fn is_inout(&self) -> bool {
        self.direction == ParamDirection::InOut
    }
}
