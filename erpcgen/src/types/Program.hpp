/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__PROGRAM_H_
#define _EMBEDDED_RPC__PROGRAM_H_

#include "DataType.hpp"
#include "Symbol.hpp"

#include <string>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen {

/*!
 * @brief Program declaration.
 *
 */
class Program : public Symbol
{
public:
    /*!
     * @brief Constructor.
     *
     * This function set symbol token to given token.
     *
     * @param[in] tok Given token.
     */
    explicit Program(const Token &tok) : Symbol(kProgramSymbol, tok) {}
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__PROGRAM_H_
