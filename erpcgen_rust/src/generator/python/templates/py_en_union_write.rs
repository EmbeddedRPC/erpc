/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use askama::Template;
use crate::generator;
use crate::generator::python::symbols::py_struct::PyEncapsulatedUnion;
use crate::generator::python::templates::util::{format_case_values_py, encode_type, optional_indent};

#[derive(Template)]
#[template(path = "python/py_en_union_write.askama")]
pub struct EnUnionWriteTemplate {
    pub encapsulated: PyEncapsulatedUnion,
}