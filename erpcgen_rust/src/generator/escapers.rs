/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


use std::fmt::Write;
use askama_escape::Escaper;

/// Escapes the given string for Python
///
/// # Arguments
///
/// * `input` - The string to escape
///
/// # Returns
///
/// The escaped string
fn py_escaper(input: &str) -> String {
    // Perform the escaping here.
    input.replace("\\", "\\\\")
}

pub struct Python;

/// Implementation of Escaper for Python
impl Escaper for Python {

    /// Writes the escaped string to the given writer
    ///
    /// # Arguments
    ///
    /// * `fmt` - The writer to write the escaped string to
    /// * `string` - The string to escape
    ///
    /// # Returns
    ///
    /// The result of writing the escaped string to the writer
    fn write_escaped<W>(&self, mut fmt: W, string: &str) -> std::fmt::Result where W: Write {
        write!(fmt, "{}", py_escaper(string))
    }
}
