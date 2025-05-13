/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/// Represents a doxygen comment.
#[derive(Debug, PartialEq, Clone)]
pub(crate) enum DoxygenComment {
    SingleLine(String),
    MultiLine(String),
}