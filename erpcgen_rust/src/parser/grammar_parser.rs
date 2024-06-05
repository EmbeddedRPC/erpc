/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use pest_derive::Parser;

#[derive(Parser)]
#[grammar = "grammar/idl_grammar.pest"]
pub struct IDLParser;