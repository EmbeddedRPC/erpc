/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


use pest::error::Error;
use pest::iterators::Pairs;
use crate::parser::expressions::expression_parser::EXPRESSION_PARSER;
use crate::parser::expressions::operation::{BinaryOperation, UnaryOperation};
use crate::parser::grammar_parser::Rule;
use crate::parser::util::expression::{map_infix, map_prefix, map_primary};
use crate::symbols::const_definition::ConstDefinition;
use crate::symbols::enum_definition::EnumDefinition;
use crate::symbols::value::Value;

/// Represents an expression tree
#[derive(Clone, Debug)]
pub(crate) enum Expression {
    Operand(Value),
    UnaryExpression(UnaryOperation, Box<Expression>),
    BinaryExpression(Box<Expression>, BinaryOperation, Box<Expression>)
}

/// Implementation of Expression
impl Expression {
    /// Creates a new expression tree from a list of pairs
    ///
    /// # Arguments
    ///
    /// * `pairs` - A list of pairs
    /// * `consts` - A list of constant definitions
    /// * `enums` - A list of enum definitions
    ///
    /// # Returns
    ///
    /// * A new expression tree.
    pub(crate) fn new_expression_tree(pairs: Pairs<Rule>, consts: &Vec<ConstDefinition>, enums: &Vec<EnumDefinition>) -> Result<Expression, Box<Error<Rule>>> {
        let tree = EXPRESSION_PARSER
            .map_primary(|primary|
                map_primary(primary, consts, enums)
            )
            .map_infix(|left, infix, right|
                map_infix(infix, left, right)
            )
            .map_prefix(|prefix, expr|
                map_prefix(prefix, expr)
            )
            .parse(pairs);
        tree
    }

    /// Evaluates the expression tree
    ///
    /// # Returns
    ///
    /// * The result of the expression.
    pub(crate) fn eval(&mut self) -> Value {
        match self {
            Expression::Operand(value) => value.clone(),
            Expression::UnaryExpression(op, operand) => {
                let operand = operand.eval();
                Expression::eval_unary(op, operand)
            }
            Expression::BinaryExpression(left, op, right) => {
                let left = left.eval();
                let right = right.eval();
                Expression::eval_binary(op, left, right)
            }
        }
    }

    /// Evaluates a unary operation
    ///
    /// # Arguments
    ///
    /// * `unary_op` - The unary operation
    /// * `operand` - The operand
    ///
    /// # Returns
    ///
    /// * The result of the operation.
    fn eval_unary(unary_op: &UnaryOperation, operand: Value) -> Value {
        match unary_op {
            UnaryOperation::Neg => -operand,
            UnaryOperation::Inv => !operand,
            UnaryOperation::Nop => operand,
        }
    }

    /// Evaluates a binary operation
    ///
    /// # Arguments
    ///
    /// * `binary_op` - The binary operation
    /// * `left` - The left operand
    /// * `right` - The right operand
    ///
    /// # Returns
    ///
    /// * The result of the operation.
    fn eval_binary(binary_op: &BinaryOperation, left: Value, right: Value) -> Value {
        match binary_op {
            BinaryOperation::Add => left + right,
            BinaryOperation::Sub => left - right,
            BinaryOperation::Mul => left * right,
            BinaryOperation::Div => left / right,
            BinaryOperation::Mod => left % right,
            BinaryOperation::Shl => left << right,
            BinaryOperation::Shr => left >> right,
            BinaryOperation::Xor => left ^ right,
            BinaryOperation::And => left & right,
            BinaryOperation::Or => left | right,
        }
    }
    
}
