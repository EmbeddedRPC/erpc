/*
 * Copyright 2024 Marek Mišík(nxf76107)
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use pest::error::{Error, ErrorVariant};
use pest::iterators::{Pair, Pairs};
use crate::parser::grammar_parser::Rule;
use crate::symbols::member_options::MemberOptions;


/// Extracts the struct definition rule from the given rules
///
/// # Arguments
///
/// * `rules` - The rules to extract the struct definition rule from
///
/// # Returns
///
/// The struct definition rule if found, otherwise None
pub(crate) fn extract_struct_member_options<'a>(rules: &'a Pairs<Rule>) -> Option<Pair<'a, Rule>> {
    rules.find_first_tagged("ol")
}

/// Parses the member options
///
/// # Arguments
///
/// * `member_option_rules` - The member option rules to parse
/// * `strict` - Whether to enforce strict parsing
///
/// # Returns
///
/// The parsed member options
pub(crate) fn parse_member_options(member_option_rules: Pairs<Rule>, strict: bool) -> Result<Vec<MemberOptions>, Box<Error<Rule>>> {
    let mut member_options = vec![];
    for member_option_rule in member_option_rules {
        match member_option_rule.as_rule() {
            Rule::byref => {
                if strict && member_options.contains(&MemberOptions::Byref) { 
                    return Err(Box::new(Error::new_from_span(ErrorVariant::CustomError {
                        message: "Duplicate byref member option".to_string()
                    }, member_option_rule.as_span())));
                }
                member_options.push(MemberOptions::Byref);
            }
            Rule::optional => {
                if strict && member_options.contains(&MemberOptions::Byref) {
                    return Err(Box::new(Error::new_from_span(ErrorVariant::CustomError {
                        message: "Duplicate nullable member option".to_string()
                    }, member_option_rule.as_span())));
                }
                member_options.push(MemberOptions::Optional);
            }
            _ => unreachable!(),
        }
    }
    Ok(member_options)
}

/// Extracts the struct definition rule from the given rules
///
/// # Arguments
///
/// * `rules` - The rules to extract the struct definition rule from
///
/// # Returns
///
/// The struct definition rule if found, otherwise None
pub(crate) fn extract_discriminator_rule<'a>(rules: &'a Pairs<Rule>) -> Option<Pair<'a, Rule>> {
    rules.find_first_tagged("disc")
}

/// Extracts the struct definition rule from the given rules
///
/// # Arguments
///
/// * `rules` - The rules to extract the struct definition rule from
///
/// # Returns
///
/// The struct definition rule if found, otherwise None
pub(crate) fn extract_options_list<'a>(rules: &'a Pairs<Rule>) -> Option<Pair<'a, Rule>> {
    rules.find_first_tagged("ol")
}

