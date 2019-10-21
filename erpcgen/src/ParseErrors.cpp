/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "ParseErrors.h"

using namespace erpcgen;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

const char *syntax_error2::what() const NOEXCEPT NOTHROW
{
    return m_what.c_str();
}
