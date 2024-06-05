/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use askama::Template;
use crate::generator::python::symbols::py_union::PyUnion;
use crate::generator::python::templates::util::{format_type_name_py, format_values_py ,format_case_values_py, decode_type, encode_type, optional_indent};
use crate::generator;
#[derive(Template)]
#[template(path = "python/py_union.askama")]
pub struct PyUnionTemplate {
    pub union_: PyUnion,
}