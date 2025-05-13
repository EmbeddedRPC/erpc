/*
 * Copyright 2024 Marek Mišík(nxf76107)
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


use pest::iterators::{Pair, Pairs};
use crate::parser::grammar_parser::Rule;


/// Extracts the interface definition rule from the given rules
///
/// # Arguments
///
/// * `rules` - The rules to extract the interface definition rule from
///
/// # Returns
///
/// The interface definition rule if found, otherwise None
pub(crate) fn extract_oneway_rule<'a>(
    rules: &'a Pairs<Rule>,
) -> Option<Pair<'a, Rule>> {
    rules.find_first_tagged("ow")
}


/// Extracts the interface definition rule from the given rules
///
/// # Arguments
///
/// * `rules` - The rules to extract the interface definition rule from
///
/// # Returns
///
/// The interface definition rule if found, otherwise None
pub(crate) fn extract_return_type_rule<'a>(
    rules: &'a Pairs<Rule>,
) -> Option<Pair<'a, Rule>> {
    rules.find_first_tagged("rt")
}


/// Extracts the interface definition rule from the given rules
///
/// # Arguments
///
/// * `rules` - The rules to extract the interface definition rule from
///
/// # Returns
///
/// The interface definition rule if found, otherwise None
pub(crate) fn extract_parameter_list_rule<'a>(
    rules: &'a Pairs<Rule>,
) -> Option<Pair<'a, Rule>> {
    rules.find_first_tagged("pl")
}

/// Extracts the interface definition rule from the given rules
///
/// # Arguments
///
/// * `rules` - The rules to extract the interface definition rule from
///
/// # Returns
///
/// The interface definition rule if found, otherwise None
pub(crate) fn extract_callback_type_definition_list_rule<'a>(
    rules: &'a Pairs<'a, Rule>
) -> Option<Pair<'a, Rule>> {
    rules.find_first_tagged("cl")
}

/// Extracts the interface definition rule from the given rules
///
/// # Arguments
///
/// * `rules` - The rules to extract the interface definition rule from
///
/// # Returns
///
/// The interface definition rule if found, otherwise None
pub(crate) fn extract_function_definition_list_rule<'a>(
    rules: &'a Pairs<Rule>,
) -> Option<Pair<'a, Rule>> {
    rules.find_first_tagged("fl")
}

/// Extracts the interface definition rule from the given rules
///
/// # Arguments
///
/// * `rules` - The rules to extract the interface definition rule from
///
/// # Returns
///
/// The interface definition rule if found, otherwise None
pub(crate) fn extract_definition<'a>(
    rules: &'a Pairs<Rule>,
) -> Option<Pair<'a, Rule>> {
    rules.find_first_tagged("def")
}

/// Extracts the interface definition rule from the given rules
///
/// # Arguments
///
/// * `rules` - The rules to extract the interface definition rule from
///
/// # Returns
///
/// The interface definition rule if found, otherwise None
pub(crate) fn extract_param_direction<'a>(
    rules: &'a Pairs<Rule>,
) -> Option<Pair<'a, Rule>> {
    rules.find_first_tagged("pd")
}

/// Extracts the interface definition rule from the given rules
///
/// # Arguments
///
/// * `rules` - The rules to extract the interface definition rule from
///
/// # Returns
///
/// The interface definition rule if found, otherwise None
pub(crate) fn extract_scope_ident<'a>(
    rules: &'a Pairs<Rule>,
) -> Option<Pair<'a, Rule>> {
    rules.find_first_tagged("scp")
}

/// Extracts the interface definition rule from the given rules
///
/// # Arguments
///
/// * `rules` - The rules to extract the interface definition rule from
///
/// # Returns
///
/// The interface definition rule if found, otherwise None
pub(crate) fn extract_simple_data_type_scope<'a>(
    rules: &'a Pairs<Rule>,
) -> Option<Pair<'a, Rule>> {
    rules.find_first_tagged("sds")
}

/// Extracts the interface definition rule from the given rules
///
/// # Arguments
///
/// * `rules` - The rules to extract the interface definition rule from
///
/// # Returns
///
/// The interface definition rule if found, otherwise None
pub(crate) fn extract_parameter_definition_list(
    rules: Pairs<Rule>,
) ->  Vec<Pair<Rule>>  {
    rules.find_tagged("pdf").collect()
}

/// Extracts the interface definition rule from the given rules
///
/// # Arguments
///
/// * `rules` - The rules to extract the interface definition rule from
///
/// # Returns
///
/// The interface definition rule if found, otherwise None
pub(crate) fn extract_void_rule<'a>(
    rules: &'a Pairs<Rule>,
) -> Option<Pair<'a, Rule>> {
    rules.find_first_tagged("vo")
}

/// Extracts the interface definition rule from the given rules
///
/// # Arguments
///
/// * `rules` - The rules to extract the interface definition rule from
///
/// # Returns
///
/// The interface definition rule if found, otherwise None
pub(crate) fn extract_callback_parameters(
    rules: Pairs<Rule>,
) -> Vec<Pair<Rule>> {
    rules.find_tagged("cbp").collect()
}
