/*
 * Copyright 2024 Marek Mišík(nxf76107)
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


use crate::parser::grammar_parser::Rule;
use pest::iterators::{Pair, Pairs};
use log::error;
use crate::symbols::doxygen_comment::DoxygenComment;


/// Extract a rule from a list of rules
///
/// # Arguments
///
/// * `rules` - A list of rules
///
/// # Returns
///
/// An optional rule
pub(crate) fn extract_doxygen_trailing_comment_tag(
    rules: &Pairs<Rule>,
) -> Option<String> {
    if let Some(rule) = rules.find_first_tagged("tc") {
        return Some(rule.as_str().to_string());
    }
    None
}

/// Extract a rule from a list of rules
///
/// # Arguments
///
/// * `rules` - A list of rules
///
/// # Returns
///
/// An optional rule
pub(crate) fn extract_annotations_list_rule_tag<'a>(
    rules: &'a Pairs<'a, Rule>,
) -> Option<Pair<'a, Rule>> {
    if let Some(rule) = rules.find_first_tagged("al") {
        return Some(rule);
    }
    None
}
/// Extract a rule from a list of rules
///
/// # Arguments
///
/// * `rules` - A list of rules
///
/// # Returns
///
/// An optional rule
pub(crate) fn extract_ident_name_tag<'a>(
    rules: &'a Pairs<Rule>,
) -> Option<Pair<'a, Rule>> {
    rules.find_first_tagged("name")
}

/// Extract a rule from a list of rules
///
/// # Arguments
///
/// * `rules` - A list of rules
///
/// # Returns
///
/// An optional rule
pub(crate) fn extract_ident_opt_name_tag<'a>(
    rules: &'a Pairs<Rule>,
) -> Option<Pair<'a, Rule>> {
    rules.find_first_tagged("opt_name")
}

/// Extract a rule from a list of rules
///
/// # Arguments
///
/// * `rules` - A list of rules
///
/// # Returns
///
/// An optional rule
pub(crate) fn extract_member_name_tag<'a>(
    rules: &'a Pairs<Rule>,
) -> Option<Pair<'a, Rule>> {
    rules.find_first_tagged("member_name")
}

/// Extract a rule from a list of rules
///
/// # Arguments
///
/// * `rules` - A list of rules
///
/// # Returns
///
/// An optional rule
pub(crate) fn extract_ident_type_tag<'a>(
    rules: &'a Pairs<Rule>,
) -> Option<Pair<'a, Rule>> {
    if let Some(rule) = rules.find_first_tagged("type") {
        return Some(rule);
    }
    None
}

/// Extract a rule from a list of rules
///
/// # Arguments
///
/// * `rules` - A list of rules
///
/// # Returns
///
/// An optional rule
pub(crate) fn extract_string_rule<'a>(
    rules: &'a Pairs<'a, Rule>
) -> Option<Pair<'a, Rule>> {
    rules.find_first_tagged("str")
}

/// Extract a rule from a list of rules
///
/// # Arguments
///
/// * `rules` - A list of rules
///
/// # Returns
///
/// An optional rule
pub(crate) fn extract_doxygen_definition_comments_preceding<'a>(
    rules: &'a Pairs<'a, Rule>
) -> Vec<DoxygenComment> {
    let Some(comments) = rules.find_first_tagged("pcd") else {
        return vec![]
    };
    parse_comments(comments)
}

/// Extract a rule from a list of rules
///
/// # Arguments
///
/// * `rules` - A list of rules
///
/// # Returns
///
/// An optional rule
pub(crate) fn extract_doxygen_member_comments_preceding<'a>(
    rules: &'a Pairs<'a, Rule>
) -> Vec<DoxygenComment> {
    let Some(comments) = rules.find_first_tagged("pc") else {
        return vec![]
    };
    parse_comments(comments)
}

/// Extract a rule from a list of rules
///
/// # Arguments
///
/// * `rules` - A list of rules
///
/// # Returns
///
/// An optional rule
pub(crate) fn extract_doxygen_definition_comments_trailing<'a>(
    rules: &'a Pairs<'a, Rule>
) -> Vec<DoxygenComment> {
    let Some(comments) = rules.find_first_tagged("tcd") else {
        return vec![]
    };
    parse_comments(comments)
}

/// Extract a rule from a list of rules
///
/// # Arguments
///
/// * `rules` - A list of rules
///
/// # Returns
///
/// An optional rule
pub(crate) fn extract_doxygen_member_comments_trailing<'a>(
    rules: &'a Pairs<'a, Rule>
) -> Vec<DoxygenComment> {
    let Some(comments) = rules.find_first_tagged("tc") else {
        return vec![]
    };
    parse_comments(comments)
}

/// Parse comments
///
/// # Arguments
///
/// * `comments` - A list of comments
///
/// # Returns
///
/// A list of doxygen comments
fn parse_comments(
    comments: Pair<Rule>
) -> Vec<DoxygenComment> {
    let comments_list = comments.into_inner();
    let mut comments_vec = vec![];
    comments_list.for_each(|comment| {
        match comment.as_rule() {
            Rule::multilineCommentContent => {
                comments_vec.push(DoxygenComment::MultiLine(comment.as_str().to_string()));

            },
            Rule::singleLineCommentContent => {
                comments_vec.push(DoxygenComment::SingleLine(comment.as_str().trim_end_matches('\n').trim_end_matches('\r').to_string()));
            },
            _ => {
                error!("{:?}", comment.as_rule());
                panic!("Unexpected rule");
            }
        }
    });
    comments_vec
}

/// Extract a rule from a list of rules
///
/// # Arguments
///
/// * `rules` - A list of rules
///
/// # Returns
///
/// An optional rule
pub(crate) fn extract_simple_data_type<'a>(
    rules: &'a Pairs<'a, Rule>
) -> Option<Pair<'a, Rule>> {
    rules.find_first_tagged("sdt")
}

/// Extract a rule from a list of rules
///
/// # Arguments
///
/// * `rules` - A list of rules
///
/// # Returns
///
/// An optional rule
pub(crate) fn extract_cases_list<'a>(
    rules: &'a Pairs<'a, Rule>
) -> Option<Pair<'a, Rule>> {
    rules.find_first_tagged("cl")
}

/// Extract a rule from a list of rules
///
/// # Arguments
///
/// * `rules` - A list of rules
///
/// # Returns
///
/// An optional rule
pub(crate) fn extract_member_list<'a>(
    rules: &'a Pairs<'a, Rule>
) -> Option<Pair<'a, Rule>> {
    rules.find_first_tagged("ml")
}
