/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


/// ForwardDefinitionRecord is a struct that holds the name of the forward definition,
/// the position of the forward definition in the file, and the file name of the file that the forward definition is in.
#[derive(Debug, Clone)]
pub(crate) struct ForwardDefinitionRecord {
    pub(crate) name: String,
    pub(crate) position: usize,
    pub(crate) file_name: String,
}

/// Implementation of ForwardDefinitionRecord
impl ForwardDefinitionRecord {
    pub(crate) fn new(name: String, position: usize, file_name: String) -> Self {
        Self {
            name,
            position,
            file_name,
        }
    }
}