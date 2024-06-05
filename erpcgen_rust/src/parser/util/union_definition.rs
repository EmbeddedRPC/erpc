/*
 * Copyright 2024 Marek Mišík(nxf76107)
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use pest::iterators::{Pair, Pairs};
use crate::parser::grammar_parser::Rule;

/// Finds the default rule
///
/// # Arguments
///
/// * `rules` - The rules to find the default rule from
///
/// # Returns
///
/// The default rule if found, otherwise None
pub(crate) fn find_default_rule<'a>(rules: &Pairs<'a, Rule>) -> Option<Pair<'a, Rule>> {
    rules.find_first_tagged("dc")
}