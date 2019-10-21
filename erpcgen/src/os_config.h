/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__OS_CONFIG_
#define _EMBEDDED_RPC__OS_CONFIG_
////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

// Macro to hide noexcept keyword for Visual C++.
#if WIN32
#define NOEXCEPT
#else
#define NOEXCEPT noexcept
#endif // NOEXCEPT

// Macro to add throw() keyword for MinGW C++.
#if __MINGW32__
#define NOTHROW throw()
#else
#define NOTHROW
#endif // NOTHROW
#endif // _EMBEDDED_RPC__OS_CONFIG_
