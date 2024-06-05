/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use askama::Template;
use crate::generator::python::symbols::py_struct::{PyStruct};
use crate::generator::python::templates::util::{format_case_values_py, encode_type, format_class_init_py, format_type_name_py, format_values_py, get_current_array_dim, shed_array_dimension, optional_indent, decode_type};
use crate::generator;
#[derive(Template)]
#[template(path = "python/py_struct.askama")]
pub struct StructTemplate {
    pub struct_: PyStruct
}
