/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use std::collections::HashMap;
use crate::symbols::struct_data_member::StructDataMember;
use crate::symbols::union_definition::UnionDefinition;
use crate::symbols::value::Value;

/// Struct member
#[derive(Debug, PartialEq, Clone)]
pub(crate) enum StructMember {
    DataMember(StructDataMember),
    UnionDefinition {
        definition: UnionDefinition,
        discriminator: Value, // Can be Value::RuntimeValue
    }, 
}

impl StructMember {
    pub fn apply_rename(&mut self, struct_renames: &HashMap<String, String>, program_renames: &HashMap<String, String>) {
        match self {
            StructMember::DataMember(member) => member.apply_rename(struct_renames, program_renames),
            StructMember::UnionDefinition { definition, .. } => definition.apply_rename_encapsulated(struct_renames, program_renames),
        }
    }
}