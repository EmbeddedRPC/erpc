/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use askama::Template;
use crate::generator::python::symbols::py_typedef::PyTypeDef;
use crate::generator;
#[derive(Template)]
#[template(path = "python/py_typedef.askama")]
pub struct PyTypeDefTemplate {
    pub typedef_: PyTypeDef
}