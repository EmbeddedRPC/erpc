/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/// Member options
#[derive(Debug, Clone, Copy, PartialEq)]
pub(crate) enum MemberOptions{
    Byref,
    Optional,
}