/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use askama::Template;
use crate::generator::python::symbols::py_struct::PyEncapsulatedUnion;
use crate::generator::python::templates::util::{format_case_values_py, decode_type, optional_indent};
use crate::generator;

#[derive(Template)]
#[template(path = "python/py_en_union_read.askama")]
pub struct EnUnionReadTemplate {
    pub encapsulated: PyEncapsulatedUnion,
}