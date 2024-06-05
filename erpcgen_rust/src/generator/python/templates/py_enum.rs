/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use askama::Template;
use crate::generator::python::symbols::py_enum::PyEnum;
use crate::generator;

#[derive(Template)]
#[template(path = "python/py_enum.askama")]
pub(crate) struct EnumTemplate {
    enum_: PyEnum
}