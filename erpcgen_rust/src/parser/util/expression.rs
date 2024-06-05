/*
 * Copyright 2024 Marek Mišík(nxf76107)
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


use log::{info, trace};
use pest::error::{Error, ErrorVariant};
use pest::iterators::{Pair, Pairs};
use std::num::{ParseFloatError, ParseIntError};
use std::str::FromStr;
use pest::Span;
use crate::parser::expressions::expression::Expression;
use crate::parser::expressions::operation::{BinaryOperation, UnaryOperation};
use crate::parser::grammar_parser::Rule;
use crate::symbols::const_definition::ConstDefinition;
use crate::symbols::enum_definition::EnumDefinition;
use crate::symbols::types::Type;
use crate::symbols::value::Value;

/// Represents the expression parser
///
/// # Arguments
///
/// * `pair` - The pair to parse
/// * `consts` - The list of constant definitions
/// * `enums` - The list of enum definitions
///
/// # Returns
///
/// The parsed expression
pub(crate) fn map_primary(pair: Pair<Rule>, consts: &Vec<ConstDefinition>, enums: &Vec<EnumDefinition>) -> Result<Expression, Box<Error<Rule>>> {
    let pos = pair.as_span().start_pos();
    let literal = pair.as_str();
    if Rule::bracedExpression == pair.as_rule() {
        let inner = pair.into_inner();
        let expr_rule = extract_expression_rule_tag(&inner).expect("Braced expression has inner expression");
        return Expression::new_expression_tree(expr_rule.into_inner(), consts, enums);
    }
    if Rule::expression == pair.as_rule() {
        return Expression::new_expression_tree(pair.into_inner(), consts, enums);
    }
    let res = match pair.as_rule() {
        Rule::decimal => {
            parse_decimal(literal)
                .map_err(|e| Box::new(Error::new_from_pos(ErrorVariant::CustomError {
                    message: e.to_string(),
                }, pos)))
        }
        Rule::hexadecimal => {
            parse_hexadecimal(literal)
                .map_err(|e| Box::new(Error::new_from_pos(ErrorVariant::CustomError {
                    message: e.to_string(),
                }, pos)))
        }
        Rule::binary => {
            parse_binary(literal)
                .map_err(|e| Box::new(Error::new_from_pos(ErrorVariant::CustomError {
                    message: e.to_string(),
                }, pos)))
        }
        Rule::floatingPoint => {
            parse_floating_point(literal)
                .map_err(|e| Box::new(Error::new_from_pos(ErrorVariant::CustomError {
                    message: e.to_string(),
                }, pos)))
        }
        Rule::ident => {
            resolve_value(literal, pair.as_span(), consts, enums, false)
        }
        _ => {
            println!("Error: {:?}", pair.as_rule());
            unreachable!("Unexpected rule")
        }
    }?;
    Ok(Expression::Operand(res))
}


/// Resolves the value of the given identifier
///
/// # Arguments
///
/// * `ident` - The identifier to resolve
/// * `span` - The span of the identifier
/// * `consts` - The list of constant definitions
/// * `enums` - The list of enum definitions
/// * `keep_reference` - Whether to keep the reference
///
/// # Returns
///
/// The value of the identifier
pub(crate) fn resolve_value(ident: &str, span: Span, consts: &[ConstDefinition], enums: &[EnumDefinition], keep_reference: bool ) -> Result<Value, Box<Error<Rule>>> {
    let const_value = consts
        .iter()
        .find(|c| c.name == ident)
        .map(|c| Value::ReferencedValue { name: c.name.clone(), prefix: None, value: Box::new(c.value.clone()) });
    if let Some(v) = const_value {
        return Ok(v)
    }
    
    let enum_value = enums
        .iter()
        .flat_map(|ed| ed.members.iter().map(move |m| (ed, m)))
        .find(|(_, m)| m.name == ident)
        .map(|(ed, m)| {
            if keep_reference {
                Value::ReferencedValue { name: m.name.clone(), prefix: ed.name.clone(), value: Box::new(m.value.clone()) }
            } else {
                m.value.clone()
            }
        });
    
    return match enum_value {
        Some(v) => {
            Ok(v)
        }
        _ => {
            Err(Box::new(Error::new_from_pos(ErrorVariant::CustomError {
                message: format!("Value {} not is not defined", ident).to_string(),
            }, span.start_pos())))
        }
    }
}

/// Maps the infix expression
///
/// # Arguments
///
/// * `pair` - The pair to map
/// * `lhs` - The left hand side expression
/// * `rhs` - The right hand side expression
///
/// # Returns
///
/// The mapped expression
pub(crate) fn map_infix(pair: Pair<Rule>, lhs: Result<Expression, Box<Error<Rule>>>, rhs: Result<Expression, Box<Error<Rule>>>) -> Result<Expression, Box<Error<Rule>>> {
    let left = lhs?;
    let right = rhs?;
    match pair.as_rule() {
        Rule::add => {
            Ok(Expression::BinaryExpression(Box::new(left), BinaryOperation::Add, Box::new(right)))
        }
        Rule::sub => {
            Ok(Expression::BinaryExpression(Box::new(left), BinaryOperation::Sub, Box::new(right)))
        }
        Rule::mul => {
            Ok(Expression::BinaryExpression(Box::new(left), BinaryOperation::Mul, Box::new(right)))
        }
        Rule::div => {
            Ok(Expression::BinaryExpression(Box::new(left), BinaryOperation::Div, Box::new(right)))
        }
        Rule::md => {
            Ok(Expression::BinaryExpression(Box::new(left), BinaryOperation::Mod, Box::new(right)))
        }
        Rule::shl => {
            Ok(Expression::BinaryExpression(Box::new(left), BinaryOperation::Shl, Box::new(right)))
        }
        Rule::shr => {
            Ok(Expression::BinaryExpression(Box::new(left), BinaryOperation::Shr, Box::new(right)))
        }
        Rule::xor => {
            Ok(Expression::BinaryExpression(Box::new(left), BinaryOperation::Xor, Box::new(right)))
        }
        Rule::and => {
            Ok(Expression::BinaryExpression(Box::new(left), BinaryOperation::And, Box::new(right)))
        }
        Rule::or => {
            Ok(Expression::BinaryExpression(Box::new(left), BinaryOperation::Or, Box::new(right)))
        }
        _ => {
            unreachable!("Unexpected rule")
        }
    }
}

/// Maps the prefix expression
///
/// # Arguments
///
/// * `pair` - The pair to map
/// * `expression` - The expression to map
///
/// # Returns
///
/// The mapped expression
pub(crate) fn map_prefix(pair: Pair<Rule>, expression: Result<Expression, Box<Error<Rule>>>) -> Result<Expression, Box<Error<Rule>>> {
    if let Ok(e) = expression {
        let res = match pair.as_rule() {
            Rule::nop => {
                e
            }
            Rule::neg => {
                Expression::UnaryExpression(UnaryOperation::Neg, Box::new(e))
            }
            Rule::inv => {
                Expression::UnaryExpression(UnaryOperation::Inv, Box::new(e))
            }
            _ => {
                unreachable!("Unexpected rule")
            }
        };
        return Ok(res);
    }
    expression
}


/// Evaluates the given expression
///
/// # Arguments
///
/// * `rule` - The rule to evaluate
/// * `target_type` - The target type
/// * `consts` - The list of constant definitions
/// * `enums` - The list of enum definitions
/// * `allow_reference` - Whether to allow reference
/// * `strict` - Whether to cast strictly
///
/// # Returns
///
/// The evaluated value
pub(crate) fn evaluate_expression(rule: Pair<Rule>, target_type: &Type, consts: &Vec<ConstDefinition>, enums: &Vec<EnumDefinition>, allow_reference: bool , strict: bool) -> Result<Value, Box<Error<Rule>>> {
    let pos = rule.as_span().start_pos();
    info!("Evaluating expression {} with target type {:?}", rule.as_str(), target_type);
    let mut pairs = rule.into_inner();
    
    if pairs.peek().is_some_and(|p| p.as_rule() == Rule::string) {
        if target_type != &Type::String { 
            return Err(Box::new(Error::new_from_pos(ErrorVariant::CustomError {
                message: format!("Cannot cast string to {:?}", target_type).to_string(),
            }, pos)))
        }
        let string = pairs.next().expect("Expected string").as_str();
        return Ok(Value::String(string.to_string()));
    }
    
    // In case of union, we want to get Value::ReferencedValue if possible
    if allow_reference && pairs.len() == 1 && pairs.peek().is_some_and(|pair| pair.as_rule() == Rule::ident) {
        info!("Resolving reference");
        let ident_rule = pairs.next().expect("Expected ident");
        return resolve_value(ident_rule.as_str(), ident_rule.as_span(), consts, enums, allow_reference);
    }
    
    let mut expression_tree = Expression::new_expression_tree(pairs, consts, enums)?;
    info!("Expression tree: {:?}", expression_tree);
    
    let value = expression_tree.eval();
    info!("Value: {:?}", value);
    let value_type = value.determine_type();
    let casted_value = cast_to(value, target_type, strict);
    trace!("Casted value: {:?}", casted_value);
    match casted_value {
        Some(v) => {
            Ok(v)
        }
        None => {
            Err(Box::new(Error::new_from_pos(ErrorVariant::CustomError {
                message: format!("Cannot cast {:?} to {:?}", value_type, target_type).to_string(),
            }, pos)))
        }
    }
}


/// Parses the decimal value
///
/// # Arguments
///
/// * `str` - The string to parse
///
/// # Returns
///
/// The parsed value
pub(crate) fn parse_decimal(str: &str) -> Result<Value, ParseIntError> {
    let pos = str.find(|c: char| c.is_alphabetic());
    let (value, suffix) = match pos {
        Some(pos) => str.split_at(pos),
        None => (str, ""),
    };
    if suffix.is_empty() {
        return Ok(Value::Int64(value.parse::<i64>()?));
    }
    Ok(Value::UInt64(value.parse::<u64>()?))
}

/// Parses the hexadecimal value
///
/// # Arguments
///
/// * `str` - The string to parse
///
/// # Returns
///
/// The parsed value
pub(crate) fn parse_hexadecimal(str: &str) -> Result<Value, ParseIntError> {
    let value = str.trim_start_matches("0x");
    let pos = value.find(|c: char| c.is_alphabetic());
    let (value, suffix) = match pos {
        Some(pos) => value.split_at(pos),
        None => (value, ""),
    };
    if suffix.is_empty() {
        return Ok(Value::Int64(i64::from_str_radix(value, 16)?));
    }
    Ok(Value::UInt64(u64::from_str_radix(value, 16)?))
}

/// Parses the binary value
///
/// # Arguments
///
/// * `str` - The string to parse
///
/// # Returns
///
/// The parsed value
pub(crate) fn parse_binary(str: &str) -> Result<Value, ParseIntError> {
    let value = str.trim_start_matches("0b");
    let pos = value.find(|c: char| c.is_alphabetic());
    let (value, suffix) = match pos {
        Some(pos) => value.split_at(pos),
        None => (value, ""),
    };
    if suffix.is_empty() {
        return Ok(Value::Int64(i64::from_str_radix(value, 2)?));
    }
    Ok(Value::UInt64(u64::from_str_radix(value, 2)?))
}

/// Parses the floating point value
///
/// # Arguments
///
/// * `str` - The string to parse
///
/// # Returns
///
/// The parsed value
pub(crate) fn parse_floating_point(str: &str) -> Result<Value, ParseFloatError> {
    Ok(Value::Double(f64::from_str(str)?))
}

/// Casts the given value to the target type
///
/// # Arguments
///
/// * `value` - The value to cast
/// * `target_type` - The target type
/// * `strict` - Whether to cast strictly
///
/// # Returns
///
/// The casted value if possible, otherwise None
pub(crate) fn cast_to(value: Value, target_type: &Type, strict: bool) -> Option<Value> {
    if let Value::ReferencedValue { value, .. } = value { 
        return cast_to(*value, target_type, strict);
    }
    if strict && value.is_integer() {
        return match target_type {
            Type::Int8 => Some(Value::Int8(value.to_i8())),
            Type::Int16 => Some(Value::Int16(value.to_i16())),
            Type::Int32 => Some(Value::Int32(value.to_i32())),
            Type::Int64 => Some(Value::Int64(value.to_i64())),
            Type::UInt8 => Some(Value::Uint8(value.to_u8())),
            Type::UInt16 => Some(Value::Uint16(value.to_u16())),
            Type::UInt32 => Some(Value::Uint32(value.to_u32())),
            _ => None,
        };
    }
    if strict && !value.is_integer() {
        return match target_type {
            Type::Float => Some(Value::Float(value.to_f32())),
            Type::Double => Some(Value::Double(value.to_f64())),
            _ => None,
        };
    }

    match target_type {
        Type::Int8 => Some(Value::Int8(value.to_i8())),
        Type::Int16 => Some(Value::Int16(value.to_i16())),
        Type::Int32 => Some(Value::Int32(value.to_i32())),
        Type::Int64 => Some(Value::Int64(value.to_i64())),
        Type::UInt8 => Some(Value::Uint8(value.to_u8())),
        Type::UInt16 => Some(Value::Uint16(value.to_u16())),
        Type::UInt32 => Some(Value::Uint32(value.to_u32())),
        Type::UInt64 => Some(Value::UInt64(value.to_u64())),
        Type::Float => Some(Value::Float(value.to_f32())),
        Type::Double => Some(Value::Double(value.to_f64())),
        _ => None,
    }
}

/// Extracts the expression rule from the given rules
///
/// # Arguments
///
/// * `rules` - The rules to extract the expression rule from
///
/// # Returns
///
/// The expression rule if found, otherwise None
pub(crate) fn extract_expression_rule_tag<'a>(
    rules: &'a Pairs<'a, Rule>,
) -> Option<Pair<'a, Rule>> {
    if let Some(rule) = rules.find_first_tagged("exp") {
        return Some(rule);
    }
    None
}

