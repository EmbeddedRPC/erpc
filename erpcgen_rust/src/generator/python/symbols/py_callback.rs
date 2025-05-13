/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

pub struct PyCallback {
    pub name: String,
    pub functions: Vec<String>
}

impl PyCallback {
    pub fn new(name: &str, functions: Vec<String>) -> Self {
        Self {
            name: name.to_string(),
            functions
        }
    }
}