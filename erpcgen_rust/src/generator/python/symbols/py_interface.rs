/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use crate::generator::python::symbols::py_function::PyFunction;
//use crate::generator::python::symbols::py_function::PyFunction;
use crate::generator::python::util::format_comments;
use crate::symbols::interface_definition::InterfaceDefinition;

pub struct PyInterface {
    pub name: String,
    pub id: usize,
    pub functions: Vec<PyFunction>,
    pub preceding_comment: String,
    pub trailing_comment: String,
}

impl PyInterface {
    pub(crate) fn from(interface: &InterfaceDefinition) -> Self {
        Self {
            name: interface.name.clone(),
            id: interface.id.expect("Interface ID must be set"),
            functions: interface.functions.iter().map(PyFunction::from).collect(),
            preceding_comment: format_comments(interface.doxygen_preceding_comment.clone()),
            trailing_comment: format_comments(interface.doxygen_trailing_comment.clone()),
        }
    }
}