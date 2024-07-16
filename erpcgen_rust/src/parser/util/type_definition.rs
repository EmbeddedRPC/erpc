/*
 * Copyright 2024 Marek Mišík(nxf76107)
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


use pest::iterators::{Pair, Pairs};
use crate::parser::grammar_parser::Rule;

/// Extracts the data type rule from the given rules
///
/// # Arguments
///
/// * `rules` - The rules to extract the data type rule from
///
/// # Returns
///
/// The data type rule if found, otherwise None
pub(crate) fn extract_data_type_rule<'a>(
    rules: &'a Pairs<'a, Rule>,
) -> Option<Pair<'a, Rule>> {
    rules.find_first_tagged("dt")
}