/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use crate::generator::python::util::{format_comments, value_to_string};
use crate::symbols::const_definition::ConstDefinition;

pub(crate) struct PyConst {
    pub name: String,
    pub value: String,
    pub preceding_comment: String,
    pub trailing_comment: String,
}

impl PyConst {
    pub(crate) fn from(const_def: &ConstDefinition) -> Self {
        Self {
            name: const_def.name.clone(),
            value: value_to_string(const_def.value.clone()),
            preceding_comment: format_comments(const_def.doxygen_preceding_comment.clone()),
            trailing_comment: format_comments(const_def.doxygen_trailing_comment.clone()),
        }
    }
}