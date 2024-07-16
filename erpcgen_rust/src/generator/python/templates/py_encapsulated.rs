/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use askama::Template;
use crate::generator::python::symbols::py_struct::{PyEncapsulatedUnion };
use crate::generator::python::templates::util::{format_class_init_py, format_type_name_py, format_values_py}; // Keep for the template
use crate::generator;

#[derive(Template)]
#[template(path = "python/py_encapsulated.askama")]
pub struct StructTemplate {
    pub encapsulated: PyEncapsulatedUnion
}