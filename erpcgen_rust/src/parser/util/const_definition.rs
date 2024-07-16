use pest::iterators::{Pair, Pairs};
use crate::parser::grammar_parser::Rule;

/// Extracts the const definition rule from the given rules
///
/// # Arguments
///
/// * `rules` - The rules to extract the const definition rule from
///
/// # Returns
///
/// The const definition rule if found, otherwise None
pub(crate) fn extract_const_expression_rule<'a>(
    rules: &'a Pairs<'a, Rule>,
) -> Option<Pair<'a, Rule>> {
    rules.find_first_tagged("ce")
}
