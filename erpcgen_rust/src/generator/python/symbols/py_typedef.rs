/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use crate::symbols::type_definition::TypeDefinition;

pub struct PyTypeDef {
    pub name: String,
    pub type_: String,
}

impl PyTypeDef {
    pub(crate) fn from(type_definition: &TypeDefinition) -> Self {
        Self {
            name: type_definition.name.clone(),
            type_: type_definition.referenced_type.get_name()
        }
    }
}