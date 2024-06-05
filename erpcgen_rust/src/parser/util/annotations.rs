/*
 * Copyright 2024 Marek Mišík(nxf76107)
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


use pest::iterators::{Pair, Pairs};
use pest::error::{Error, ErrorVariant};
use crate::parser::grammar_parser::Rule;
use crate::parser::util::common::{extract_ident_name_tag, extract_string_rule};
use crate::parser::util::expression::extract_expression_rule_tag;
use crate::symbols::annotations::Annotation;
use crate::symbols::interface_definition::InterfaceDefinitionBuilder;
use crate::symbols::language::Language;
use crate::symbols::pending_ident::{CHECK_SCALAR, PendingIdent};
use crate::symbols::program::Program;
use crate::symbols::types::Type;
use crate::symbols::value::Value;

/// Extracts the annotations rule from the given rules.
///
/// # Arguments
///
/// * `pair` - The pair containing the annotations rule.
///
/// # Returns
///
/// The annotations rule.
pub(crate) fn parse_include_annotation(pair: Pair<Rule>) -> Annotation {
    let rules = pair.into_inner();
    let language_specifier: Option<Language> = extract_language_specifier(&rules);
    let include_path: Option<String> = extract_string_rule(&rules).map(|rule| rule.as_str().to_string());
    Annotation::Include {
        language_specifier,
        path: include_path.unwrap_or_default(),
    }
}

/// Extracts the annotations rule from the given rules.
///
/// # Arguments
///
/// * `pair` - The pair containing the annotations rule.
///
/// # Returns
///
/// The annotations rule.
pub(crate) fn parse_no_alloc_errors_annotation(pair: Pair<Rule>) -> Annotation {
    let rules = pair.into_inner();
    let language_specifier: Option<Language> = extract_language_specifier(&rules);
    Annotation::NoAllocErrors { language_specifier }
}

/// Extracts the annotations rule from the given rules.
///
/// # Arguments
///
/// * `pair` - The pair containing the annotations rule.
///
/// # Returns
///
/// The annotations rule.
pub(crate) fn parse_no_const_param_annotation(pair: Pair<Rule>) -> Annotation {
    let rules = pair.into_inner();
    let language_specifier: Option<Language> = extract_language_specifier(&rules);
    Annotation::NoConstParam { language_specifier }
}

/// Extracts the annotations rule from the given rules.
///
/// # Arguments
///
/// * `pair` - The pair containing the annotations rule.
///
/// # Returns
///
/// The annotations rule.
pub(crate) fn parse_no_infra_errors_annotation(pair: Pair<Rule>) -> Annotation {
    let rules = pair.into_inner();
    let language_specifier: Option<Language> = extract_language_specifier(& rules);
    Annotation::NoInfraErrors { language_specifier }
}

/// Extracts the annotations rule from the given rules.
///
/// # Arguments
///
/// * `pair` - The pair containing the annotations rule.
///
/// # Returns
///
/// The annotations rule.
pub(crate) fn parse_output_dir_annotation(pair: Pair<Rule>) -> Annotation {
    let rules = pair.into_inner();
    let language_specifier: Option<Language> = extract_language_specifier(&rules);
    let output_dir: Option<String> = extract_string_rule(&rules).map(|rule| rule.as_str().to_string());
    Annotation::OutputDir {
        language_specifier,
        path: output_dir.unwrap_or_default(),
    }
}

/// Extracts the annotations rule from the given rules.
///
/// # Arguments
///
/// * `pair` - The pair containing the annotations rule.
///
/// # Returns
///
/// The annotations rule.
pub(crate) fn parse_types_header_annotation(pair: Pair<Rule>) -> Annotation {
    let rules = pair.into_inner();
    let language_specifier: Option<Language> = extract_language_specifier(&rules);
    Annotation::TypesHeader { language_specifier }
}

/// Extracts annotations rule from the given rules.
///
/// # Arguments
///
/// * `pair` - The pair containing the annotations rule.
///
/// # Returns
///
/// The annotations rule.
pub(crate) fn parse_nullable_annotation(pair: Pair<Rule>) -> Annotation {
    let rules = pair.into_inner();
    let language_specifier: Option<Language> = extract_language_specifier(&rules);
    Annotation::Nullable {
        language_specifier
    }
}

/// Extracts annotations rule from the given rules.
///
/// # Arguments
///
/// * `pair` - The pair containing the annotations rule.
///
/// # Returns
///
/// The annotations rule.
pub(crate) fn parse_retain_annotation(pair: Pair<Rule>) -> Annotation {
    let rules = pair.into_inner();
    let language_specifier: Option<Language> = extract_language_specifier(&rules);
    Annotation::Retain {
        language_specifier
    }
}

/// Extracts annotations rule from the given rules.
///
/// # Arguments
///
/// * `pair` - The pair containing the annotations rule.
///
/// # Returns
///
/// The annotations rule.
pub(crate) fn parse_name_annotation(pair: Pair<Rule>) -> Annotation {
    let rules = pair.into_inner();
    let language_specifier: Option<Language> = extract_language_specifier(&rules);
    let name: Option<String> = extract_string_rule(&rules).map(|rule| rule.as_str().trim_start_matches('"').trim_end_matches('"').to_string());
    Annotation::Name {
        language_specifier,
        name: name.expect("Name annotation must have a name."),
    }
}

/// Extracts language specifier from the given string.
///
/// # Arguments
///
/// * `str` - The string containing the language specifier.
///
/// # Returns
///
/// The language specifier.
pub(crate) fn resolve_language_specifier(str: &str) -> Option<Language> {
    match str {
        "c" => Some(Language::C),
        "py" => Some(Language::Python),
        "java" => Some(Language::Java),
        "rust" => Some(Language::Rust),
        _ => None,
    }
}

/// Extracts annotations rule from the given rules.
///
/// # Arguments
///
/// * `pair` - The pair containing the annotations rule.
///
/// # Returns
///
/// The annotations rule.
pub(crate) fn parse_crc_annotation(pair: Pair<Rule>) -> Annotation {
    let rules = pair.into_inner();
    let language_specifier: Option<Language> = extract_language_specifier(&rules);
    Annotation::Crc {
        language_specifier,
    }
}

/// Extracts annotations rule from the given rules.
///
/// # Arguments
///
/// * `pair` - The pair containing the annotations rule.
///
/// # Returns
///
/// The annotations rule.
pub(crate) fn parse_group_annotation(pair: Pair<Rule>) -> Annotation {
    let rules = pair.into_inner();
    let language_specifier: Option<Language> = extract_language_specifier(&rules);
    let group: Option<String> = extract_string_rule(&rules).map(|rule| rule.as_str().to_string());
    Annotation::Group {
        language_specifier,
        group: group.expect("Group annotation must have a group."),
    }
}

/// Extracts language specifier from the given rules.
///
/// # Arguments
///
/// * `rules` - The rules containing the language specifier.
///
/// # Returns
///
/// The language specifier.
pub(crate) fn extract_language_specifier<'a>(rules: &'a Pairs<'a, Rule>) -> Option<Language> {
    rules.find_first_tagged("ls").map(|language| {
        resolve_language_specifier(language.as_str()).expect("Invalid language specifier")
    })
}

/// Extracts annotations rule from the given rules.
///
/// # Arguments
///
/// * `pair` - The pair containing the annotations rule.
///
/// # Returns
///
/// The annotations rule.
pub(crate) fn parse_external_annotation(pair: Pair<Rule>) -> Annotation {
    let rules = pair.into_inner();
    let language_specifier: Option<Language> = extract_language_specifier(&rules);
    Annotation::External {
        language_specifier,
    }
}

/// Extracts annotations rule from the given rules.
///
/// # Arguments
///
/// * `pair` - The pair containing the annotations rule.
///
/// # Returns
///
/// The annotations rule.
pub(crate) fn parse_shared_annotation(pair: Pair<Rule>) -> Annotation {
    let rules = pair.into_inner();
    let language_specifier: Option<Language> = extract_language_specifier(&rules);
    Annotation::Shared {
        language_specifier,
    }
}

/// Extracts annotations rule from the given rules.
///
/// # Arguments
///
/// * `program` - The program containing the annotations rule.
/// * `rule` - The pair containing the annotations rule.
///
/// # Returns
///
/// The annotations rule.
fn parse_shared_memory_begin_annotation(program: &Program, rule: Pair<'_, Rule>) -> Result<Annotation, Box<Error<Rule>>> {
    let rules = rule.into_inner();
    let language_specifier: Option<Language> = extract_language_specifier(&rules);
    let address = program.resolve_value_from_expression(extract_expression_rule_tag(&rules).expect("Expected expression"), &Type::UInt32, false)?;

    Ok(Annotation::SharedMemoryBegin {
        language_specifier,
        address: address.to_u64() as usize,
    })
}

/// Extracts annotations rule from the given rules.
///
/// # Arguments
///
/// * `program` - The program containing the annotations rule.
/// * `rule` - The pair containing the annotations rule.
///
/// # Returns
///
/// The annotations rule.
fn parse_shared_memory_end_annotation(program: &Program, rule: Pair<'_, Rule>) -> Result<Annotation, Box<Error<Rule>>> {
    let rules = rule.into_inner();
    let language_specifier: Option<Language> = extract_language_specifier(&rules);
    let address = program.resolve_value_from_expression(extract_expression_rule_tag(&rules).expect("Expected expression"), &Type::UInt32, false)?;

    Ok(Annotation::SharedMemoryEnd {
        language_specifier,
        address: address.to_u64() as usize,
    })
}

/// Extracts annotations rule from the given rules.
///
/// # Arguments
///
/// * `program` - The program containing the annotations rule.
/// * `pair` - The pair containing the annotations rule.
/// * `check_id_is_free` - The function to check if the ID is free.
///
/// # Returns
///
/// The annotations rule.
pub(crate) fn parse_id_annotation(program: &Program, pair: Pair<Rule>, check_id_is_free: impl Fn(usize) -> bool) -> Result<Annotation, Box<Error<Rule>>> {
    let rules = pair.into_inner();
    let language_specifier: Option<Language> = extract_language_specifier(&rules);
    let id_rule = extract_expression_rule_tag(&rules).expect("Expected expression");
    let id = program.resolve_value_from_expression(id_rule.clone(), &Type::UInt64, false)?;
    if !check_id_is_free(id.to_u64() as usize) {
        return Err(
            Box::new(Error::new_from_span(
                ErrorVariant::CustomError {
                    message: format!("ID {} is already in use", id.to_u64() as usize),
                },
                id_rule.as_span(),
            ))
        )   
    }
    Ok(Annotation::Id {
        language_specifier,
        id: id.to_u64() as usize,
    })
}

/// Extracts annotations rule from the given rules.
///
/// # Arguments
///
/// * `pair` - The pair containing the annotations rule.
///
/// # Returns
///
/// The annotations rule.
pub(crate) fn parse_discriminator_annotation(pair: Pair<Rule>) -> (Annotation, PendingIdent) {
    let rules = pair.clone().into_inner();
    let language_specifier: Option<Language> = extract_language_specifier(&rules);
    let discriminator_rule = extract_ident_name_tag(&rules).expect("Expected identifier");
    let discriminator: String = discriminator_rule.as_str().to_string();
    (Annotation::Discriminator {
        language_specifier,
        value: Value::RuntimeValue { name: discriminator.clone() },
    }, PendingIdent::new(discriminator, discriminator_rule.as_span().start_pos().pos(), Box::new(CHECK_SCALAR)))
}

/// Extracts annotations rule from the given rules.
///
/// # Arguments
///
/// * `program` - The program containing the annotations rule.
/// * `pair` - The pair containing the annotations rule.
///
/// # Returns
///
/// The annotations rule.
pub(crate) fn parse_error_return_annotation(program: &Program, pair: Pair<Rule>) -> Annotation {
    let rules = pair.into_inner();
    let language_specifier: Option<Language> = extract_language_specifier(&rules);
    let value = program.resolve_value_from_expression(extract_expression_rule_tag(&rules).expect("Expected expression"), &Type::Int32, false).expect("Expected integer value");
    Annotation::ErrorReturn {
        language_specifier,
        value: value.to_i32(), // TODO: This is a temporary solution
    }
}

/// Extracts annotations rule from the given rules.
///
/// # Arguments
///
/// * `program` - The program containing the annotations rule.
/// * `pair` - The pair containing the annotations rule.
///
/// # Returns
///
/// The annotations rule.
pub(crate) fn parse_length_annotation(program: &Program, pair: Pair<Rule>) -> Result<(Annotation, Option<PendingIdent>), Box<Error<Rule>>> {
    let rules = pair.into_inner();
    let language_specifier: Option<Language> = extract_language_specifier(&rules);
    let expr = extract_expression_rule_tag(&rules).expect("Expected expression");
    if let Ok(length ) = program.resolve_value_from_expression(expr.clone(), &Type::UInt32, false) {
        return Ok((Annotation::Length {
            language_specifier,
            length,
        }, None));
    };
    let Some(Rule::ident) = expr.clone().into_inner().next().map(|rule| rule.as_rule()) else { 
        return Err(Box::new(Error::new_from_span(ErrorVariant::CustomError {
                message: format!("Expected an identifier, found {:?}", expr.as_str()).to_string(),
            },
            expr.as_span(),
        )));
    };
    Ok((Annotation::Length {
        language_specifier,
        length: Value::RuntimeValue { name: expr.as_str().to_string() },
    }, Some(PendingIdent::new(expr.as_str().to_string(), expr.as_span().start_pos().pos(), Box::new(CHECK_SCALAR)))))
}

/// Extracts annotations rule from the given rules.
///
/// # Arguments
///
/// * `program` - The program containing the annotations rule.
/// * `pair` - The pair containing the annotations rule.
///
/// # Returns
///
/// The annotations rule.
pub(crate) fn parse_max_length_annotation(program: &Program, pair: Pair<Rule>) -> Result<(Annotation, Option<PendingIdent>), Box<Error<Rule>>> {
    let rules = pair.into_inner();
    let language_specifier: Option<Language> = extract_language_specifier(&rules);
    let expr = extract_expression_rule_tag(&rules).expect("Expected expression");
    if let Ok(length ) = program.resolve_value_from_expression(expr.clone(), &Type::UInt32, false) {
        return Ok((Annotation::MaxLength {
            language_specifier,
            length,
        }, None));
    };
    let Some(Rule::ident) = expr.clone().into_inner().next().map(|rule| rule.as_rule()) else { 
        return Err(Box::new(Error::new_from_span(
            ErrorVariant::CustomError {
                message: format!("Expected an identifier, found {:?}", expr.as_str()).to_string(),
            },
            expr.as_span(),
        )));
    };
    Ok((Annotation::MaxLength {
        language_specifier,
        length: Value::RuntimeValue { name: expr.as_str().to_string() },
    }, Some(PendingIdent::new(expr.as_str().to_string(), expr.as_span().start_pos().pos(), Box::new(CHECK_SCALAR)))))
}

/// Extracts annotations rule from the given rules.
///
/// # Arguments
///
/// * `struct_annotations_rule` - The pair containing the annotations rule.
/// * `language` - The language of the annotations.
/// * `strict` - The flag to indicate if the parsing should be strict.
///
/// # Returns
///
/// The annotations rule.
pub(crate) fn parse_struct_annotations(struct_annotations_rule: Pair<Rule>,language: &Language, strict: bool) -> Result<Vec<Annotation>, Box<Error<Rule>>> {
    let mut annotations = vec![];
    let annotation_rules = struct_annotations_rule.into_inner();
    for annotation in annotation_rules  {
        let ann = match annotation.as_rule() {
            Rule::nullable => parse_nullable_annotation(annotation),
            Rule::retain => parse_retain_annotation(annotation),
            Rule::name => parse_name_annotation(annotation),
            _ => {
                if strict {
                    return Err(Box::new(Error::new_from_span(
                        ErrorVariant::CustomError {
                            message: format!("Unexpected annotation: {}", annotation.as_str()).to_string(),
                        },
                        annotation.as_span(),
                    )));
                } else {
                    continue;
                }
            },
        };
        if !ann.get_language_specifier().is_some_and(|sp| &sp != language) {
            annotations.push(ann); // Filter out annotations that are not for the current language
        }
    }
    Ok(annotations)
}

/// Extracts annotations rule from the given rules.
///
/// # Arguments
///
/// * `type_annotations_rule` - The pair containing the annotations rule.
/// * `language` - The language of the annotations.
/// * `strict` - The flag to indicate if the parsing should be strict.
///
/// # Returns
///
/// The annotations rule.
pub(crate) fn parse_typedef_annotations(type_annotations_rule: Pair<Rule>,language: &Language, strict: bool) -> Result<Vec<Annotation>, Box<Error<Rule>>> {
    let mut annotations = vec![];
    let annotation_rules = type_annotations_rule.into_inner();
    for annotation in annotation_rules  {
        let ann = match annotation.as_rule() {
            Rule::external => parse_external_annotation(annotation),
            Rule::name => parse_name_annotation(annotation),
            Rule::noConstParam => parse_no_const_param_annotation(annotation),
            Rule::shared => parse_shared_annotation(annotation),
            _ => { 
                if strict { 
                    return Err(Box::new(Error::new_from_span(
                        ErrorVariant::CustomError {
                            message: format!("Unexpected annotation: {}", annotation.as_str()),
                        },
                        annotation.as_span(),
                    )));
                } else {
                    continue;
                }
            },
        };
        if !ann.get_language_specifier().is_some_and(|sp| &sp != language) {
            annotations.push(ann); // Filter out annotations that are not for the current language
        }
    }
    Ok(annotations)
} 

/// Extracts annotations rule from the given rules.
///
/// # Arguments
///
/// * `enum_annotations_rule` - The pair containing the annotations rule.
/// * `language` - The language of the annotations.
/// * `strict` - The flag to indicate if the parsing should be strict.
///
/// # Returns
///
/// The annotations rule.
pub(crate) fn parse_enum_annotations(enum_annotations_rule: Pair<Rule>, language: &Language, strict: bool) -> Result<Vec<Annotation>, Box<Error<Rule>>> {
    let mut annotations = vec![];
    let annotation_rules = enum_annotations_rule.into_inner();
    for annotation in annotation_rules  {
        let ann = match annotation.as_rule() {
            Rule::name => parse_name_annotation(annotation),
            _ => { 
                if strict { 
                    return Err(Box::new(Error::new_from_span(
                        ErrorVariant::CustomError {
                            message: format!("Unexpected annotation: {}", annotation.as_str()).to_string(),
                        },
                        annotation.as_span(),
                    )));
                } else {
                    continue;
                }
            },
        };
        if !ann.get_language_specifier().is_some_and(|sp| &sp != language) {
            annotations.push(ann); // Filter out annotations that are not for the current language
        }
    }
    Ok(annotations)
}

pub(crate) fn parse_union_annotations(union_annotations_rule: Pair<Rule>,language: &Language, strict: bool) -> Result<(Vec<Annotation>, Vec<PendingIdent>), Box<Error<Rule>>> { //TODO
    let mut annotations = vec![];
    let mut pending_idents = vec![];
    let annotation_rules = union_annotations_rule.into_inner();
    for annotation in annotation_rules  {
        let anno = match annotation.as_rule() {
            Rule::name => parse_name_annotation(annotation),
            Rule::discriminator => {
                let (ann, pend) = parse_discriminator_annotation(annotation);
                pending_idents.push(pend);
                ann
            },
            Rule::nullable => parse_nullable_annotation(annotation),
            Rule::retain => parse_retain_annotation(annotation),
            _ => {
                if strict {
                    return Err(Box::new(Error::new_from_span(
                        pest::error::ErrorVariant::CustomError {
                            message: format!("Unexpected annotation: {}", annotation.as_str()).to_string(),
                        },
                        annotation.as_span(),
                    )));
                } else {
                    continue;
                }
            },
        };
        if !anno.get_language_specifier().is_some_and(|sp| &sp != language) {
            annotations.push(anno); // Filter out annotations that are not for the current language
        }
    }
    Ok((annotations, pending_idents))
}

/// Extracts annotations rule from the given rules.
///
/// # Arguments
///
/// * `program` - The program containing the annotations rule.
/// * `list_annotations_rule` - The pair containing the annotations rule.
/// * `language` - The language of the annotations.
/// * `strict` - The flag to indicate if the parsing should be strict.
///
/// # Returns
///
/// The annotations rule.
pub(crate) fn parse_list_annotations(program: &Program, list_annotations_rule: Pair<Rule>,language: &Language, strict: bool) -> Result<(Vec<Annotation>, Vec<PendingIdent>), Box<Error<Rule>>>{
    let mut annotations = vec![];
    let mut pending_idents = vec![];
    let annotation_rules = list_annotations_rule.into_inner();
    for annotation in annotation_rules  {
        let ann = match annotation.as_rule() {
            Rule::length => {
                let (ann, pend) = parse_length_annotation(program, annotation)?;
                if let Some(p) = pend { 
                    pending_idents.push(p);
                }
                ann
            },
            Rule::maxLength => {
                let (ann, pend) = parse_max_length_annotation(program, annotation)?;
                if let Some(p) = pend {
                    pending_idents.push(p);
                }
                ann
            },
            Rule::nullable => parse_nullable_annotation(annotation),
            Rule::retain => parse_retain_annotation(annotation),
            Rule::name => parse_name_annotation(annotation),
            _ => {
                if strict {
                    return Err(Box::new(Error::new_from_span(
                        ErrorVariant::CustomError {
                            message: format!("Unexpected annotation: {}", annotation.as_str()).to_string(),
                        },
                        annotation.as_span(),
                    )));
                } else {
                    continue;
                }
            },
        };
        if !ann.get_language_specifier().is_some_and(|sp| &sp != language) {
            annotations.push(ann); // Filter out annotations that are not for the current language
        }
    }
    Ok((annotations, pending_idents))
}

/// Extracts annotations rule from the given rules.
///
/// # Arguments
///
/// * `array_annotations` - The pair containing the annotations rule.
/// * `language` - The language of the annotations.
/// * `strict` - The flag to indicate if the parsing should be strict.
///
/// # Returns
///
/// The annotations rule.
pub(crate) fn parse_array_annotations(array_annotations: Pair<'_, Rule>,language: &Language, strict: bool) -> Result<Vec<Annotation>, Box<Error<Rule>>> {
    let mut annotations = vec![];
    let annotation_rules = array_annotations.into_inner();
    for annotation in annotation_rules  {
        let ann = match annotation.as_rule() {
            Rule::retain => parse_retain_annotation(annotation),
            Rule::name => parse_name_annotation(annotation),
            _ => {
                if strict {
                    return Err(Box::new(Error::new_from_span(
                        ErrorVariant::CustomError {
                            message: format!("Unexpected annotation: {}", annotation.as_str()).to_string(),
                        },
                        annotation.as_span(),
                    )));
                } else {
                    continue;
                }
            },
        };
        if !ann.get_language_specifier().is_some_and(|sp| &sp != language) {
            annotations.push(ann); // Filter out annotations that are not for the current language
        }
    }
    Ok(annotations)
}


/// Extracts annotations rule from the given rules.
///
/// # Arguments
///
/// * `program` - The program containing the annotations rule.
/// * `program_annotations_rule` - The pair containing the annotations rule.
/// * `language` - The language of the annotations.
/// * `strict` - The flag to indicate if the parsing should be strict.
///
/// # Returns
///
/// The annotations rule.
pub(crate) fn parse_program_annotations(
    program: &Program,
    program_annotations_rule: Pair<'_, Rule>,
    language: &Language,
    strict: bool,
) -> Result<Vec<Annotation>, Box<Error<Rule>>> {
    let mut annotations = vec![];
    let annotation_rules = program_annotations_rule.into_inner();
    for annotation in annotation_rules {
        let ann = match annotation.as_rule() {
            Rule::crc => parse_crc_annotation(annotation),
            Rule::include => parse_include_annotation(annotation),
            Rule::noAllocErrors => {
                parse_no_alloc_errors_annotation(annotation)
            }
            Rule::noConstParam => parse_no_const_param_annotation(annotation),
            Rule::noInfraErrors => {
                parse_no_infra_errors_annotation(annotation)
            }
            Rule::outputDir => parse_output_dir_annotation(annotation),
            Rule::sharedMemoryBegin => {
                parse_shared_memory_begin_annotation(program, annotation)?
            }
            Rule::sharedMemoryEnd => {
                parse_shared_memory_end_annotation(program, annotation)?
            }
            Rule::typesHeader => parse_types_header_annotation(annotation),
            _ => {
                if strict { 
                    return Err(Box::new(Error::new_from_span(
                        ErrorVariant::CustomError {
                            message: format!("Unexpected annotation: {}", annotation.as_str()).to_string(),
                        },
                        annotation.as_span(),
                    )));
                } else {
                    continue;
                }
            }
        };
        if !ann.get_language_specifier().is_some_and(|sp| &sp != language) {
            annotations.push(ann); // Filter out annotations that are not for the current language
        }
    }
    Ok(annotations)
}

/// Extracts annotations rule from the given rules.
///
/// # Arguments
///
/// * `program` - The program containing the annotations rule.
/// * `interface_annotations_rule` - The pair containing the annotations rule.
/// * `language` - The language of the annotations.
/// * `strict` - The flag to indicate if the parsing should be strict.
///
/// # Returns
///
/// The annotations rule.
pub(crate) fn parse_interface_annotations(program: &Program, interface_annotations_rule: Pair<Rule>,language: &Language, strict: bool) -> Result<Vec<Annotation>, Box<Error<Rule>>> {
    let mut annotations = vec![];
    let check_id = |id| program.check_is_interface_id_free(id);
    let annotation_rules = interface_annotations_rule.into_inner();
    for annotation in annotation_rules  {
        let ann = match annotation.as_rule() {
            Rule::group => parse_group_annotation(annotation),
            Rule::include => parse_include_annotation(annotation),
            Rule::id => parse_id_annotation(program, annotation, check_id)?,
            Rule::name => parse_name_annotation(annotation),
            _ => {
                if strict {
                    return Err(Box::new(Error::new_from_span(
                        ErrorVariant::CustomError {
                            message: format!("Unexpected annotation: {}", annotation.as_str()).to_string(),
                        },
                        annotation.as_span(),
                    )));
                } else {
                    continue;
                }
            },
        };
        if !ann.get_language_specifier().is_some_and(|sp| &sp != language) {
            annotations.push(ann); // Filter out annotations that are not for the current language
        }
    }
    Ok(annotations)
}


/// Extracts annotations rule from the given rules.
///
/// # Arguments
///
/// * `program` - The program containing the annotations rule.
/// * `interface_definition_builder` - The interface definition builder.
/// * `interface_annotations_rule` - The pair containing the annotations rule.
/// * `language` - The language of the annotations.
/// * `strict` - The flag to indicate if the parsing should be strict.
///
/// # Returns
///
/// The annotations rule.
pub(crate) fn parse_function_annotations(program: &Program, interface_definition_builder: &InterfaceDefinitionBuilder, interface_annotations_rule: Pair<Rule>,language: &Language, strict: bool) -> Result<Vec<Annotation>, Box<Error<Rule>>> {
    let mut annotations = vec![];
    let annotation_rules = interface_annotations_rule.into_inner();
    let check_id = |id| interface_definition_builder.check_function_id_is_free(id);
    for annotation in annotation_rules {
        let ann = match annotation.as_rule() {
            Rule::errorReturn => parse_error_return_annotation(program, annotation),
            Rule::external => parse_external_annotation(annotation),
            Rule::id => parse_id_annotation(program, annotation, check_id)?,
            Rule::name => parse_name_annotation(annotation),
            Rule::noConstParam => parse_no_const_param_annotation(annotation),
            _ => {
                if strict {
                    return Err(Box::new(Error::new_from_span(
                        pest::error::ErrorVariant::CustomError {
                            message: format!("Unexpected annotation: {}", annotation.as_str()).to_string(),
                        },
                        annotation.as_span(),
                    )));
                } else {
                    continue;
                }
            },
        };
        if !ann.get_language_specifier().is_some_and(|sp| &sp != language) {
            annotations.push(ann); // Filter out annotations that are not for the current language
        }
    }
    Ok(annotations)
}

/// Parses the member annotations from the given rule.
///
/// This function iterates over the annotations in the given rule and parses them.
/// If the annotation is a name annotation, it is parsed using the `parse_name_annotation` function.
/// If the annotation is not recognized and the `strict` parameter is set to true, an error is returned.
/// If the annotation is not recognized and the `strict` parameter is set to false, the annotation is skipped.
/// After parsing an annotation, if the language specifier of the annotation does not match the provided language,
/// the annotation is added to the list of annotations to be returned.
///
/// # Arguments
///
/// * `array_annotations` - The pair containing the annotations rule.
/// * `language` - The language of the annotations.
/// * `strict` - The flag to indicate if the parsing should be strict.
///
/// # Returns
///
/// The annotations rule.
pub(crate) fn parse_member_annotations(array_annotations: Pair<'_, Rule>,language: &Language, strict: bool) -> Result<Vec<Annotation>, Box<Error<Rule>>> {
    let mut annotations = vec![];
    let annotation_rules = array_annotations.into_inner();
    for annotation in annotation_rules {
        let ann = match annotation.as_rule() {
            Rule::name => parse_name_annotation(annotation),
            _ => {
                if strict {
                    return Err(Box::new(Error::new_from_span(
                        ErrorVariant::CustomError {
                            message: format!("Unexpected annotation: {}", annotation.as_str()).to_string(),
                        },
                        annotation.as_span(),
                    )));
                } else {
                    continue;
                }
            },
        };
        if !ann.get_language_specifier().is_some_and(|sp| &sp != language) {
            annotations.push(ann); // Filter out annotations that are not for the current language
        }
    }
    Ok(annotations)
}


/// Checks if the given annotations rule has a discriminator annotation.
///
/// # Arguments
///
/// * `annotations` - The annotations rule.
///
/// # Returns
///
/// * `true` if the annotations rule has a discriminator annotation, otherwise `false`.
pub(crate) fn has_discriminator_annotation(annotations: &[Annotation]) -> bool {
    annotations.iter().any(|ann| matches!(ann, Annotation::Discriminator { .. }))
}

/// Finds name annotation in the given annotations rule.
///
/// # Arguments
///
/// * `annotations` - The annotations rule.
///
/// # Returns
///
/// The name annotation.
pub(crate) fn find_name_annotation(annotations: &[Annotation]) -> Option<&Annotation> {
    annotations.iter().find(|ann| matches!(ann, Annotation::Name { .. }))
}





