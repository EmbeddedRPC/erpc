/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


use lazy_static::lazy_static;
use pest::pratt_parser::PrattParser;
use crate::parser::grammar_parser::Rule;
use pest::pratt_parser::{Assoc::*, Op};

/// The expression parser
lazy_static!{
    pub(crate) static ref EXPRESSION_PARSER: PrattParser<Rule> = {
        PrattParser::new()
        .op(Op::prefix(Rule::nop) | Op::prefix(Rule::neg) | Op::prefix(Rule::inv))
        .op(Op::infix(Rule::mul, Left) | Op::infix(Rule::div, Left) | Op::infix(Rule::md, Left))
        .op(Op::infix(Rule::add, Left) | Op::infix(Rule::sub, Left))
        .op(Op::infix(Rule::shl, Left) | Op::infix(Rule::shr, Left))
        .op(Op::infix(Rule::and, Left))
        .op(Op::infix(Rule::xor, Left))
        .op(Op::infix(Rule::or, Left))
    };
}
