/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#if !defined(_format_string_h_)
#define _format_string_h_

#include <stdexcept>
#include <string>

/*!
 * \brief Returns a formatted STL string using printf format strings.
 */
std::string format_string(const char *fmt, ...);

#endif // _format_string_h_
