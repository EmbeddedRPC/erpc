/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use askama::Template;
use crate::generator::python::symbols::py_const::PyConst;
use crate::generator;

#[derive(Template)]
#[template(path = "python/py_const.askama")]
pub(crate) struct ConstTemplate {
    pub const_: PyConst
}