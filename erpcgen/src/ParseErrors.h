/*
 * The Clear BSD License
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 *  that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _EMBEDDED_RPC__PARSEERRORS_H
#define _EMBEDDED_RPC__PARSEERRORS_H

#include "Logging.h"
#include "Token.h"
#include "os_config.h"
#include <stdexcept>

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

#define MAX_MESSAGE_SIZE 100

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen {

class erpc_error : public std::runtime_error
{
public:
    explicit erpc_error(const std::string &__arg)
    : std::runtime_error(__arg)
    , m_message(__arg)
    {
    }

protected:
    std::string m_message;
    std::string m_errName;

    explicit erpc_error(const std::string &__arg, std::string errorName)
    : std::runtime_error(__arg)
    , m_message(__arg)
    , m_errName(errorName)
    {
    }
};

class syntax_error2 : public erpc_error
{
public:
    explicit syntax_error2(const std::string &__arg, token_loc_t loc, std::string &fileName)
    : erpc_error(__arg, "syntax error")
    , m_errLoc(loc)
    , m_what(format_string("file %s:%d:%d: %s, %s",
                           fileName.c_str(),
                           m_errLoc.m_firstLine,
                           m_errLoc.m_firstChar,
                           m_errName.c_str(),
                           m_message.c_str()))
    {
    }

    explicit syntax_error2(const char *__arg, token_loc_t loc, std::string &fileName)
    : erpc_error(std::string(__arg), "syntax error")
    , m_errLoc(loc)
    , m_what(format_string("file %s:%d:%d: %s, %s",
                           fileName.c_str(),
                           m_errLoc.m_firstLine,
                           m_errLoc.m_firstChar,
                           m_errName.c_str(),
                           m_message.c_str()))
    {
    }

    virtual const char *what() const NOEXCEPT NOTHROW;

private:
    token_loc_t m_errLoc;
    std::string m_what;
};

/*!
 * @brief Exception class for syntax errors.
 */
class syntax_error : public erpc_error
{
public:
    explicit syntax_error(const std::string &__arg)
    : erpc_error(__arg)
    {
    }
};

/*!
 * @brief Exception class for lexical errors.
 */
class lexical_error : public erpc_error
{
public:
    explicit lexical_error(const std::string &__arg)
    : erpc_error(__arg)
    {
    }
};

/*!
 * @brief Exception class for semantic errors.
 */
class semantic_error : public erpc_error
{
public:
    explicit semantic_error(const std::string &__arg)
    : erpc_error(__arg)
    {
    }
};

/*!
 * @brief Exception class for internal errors.
 */
class internal_error : public erpc_error
{
public:
    explicit internal_error(const std::string &__arg)
    : erpc_error(__arg)
    {
    }
};

/*!
 * @brief Throws internal_error exception on assertion failure.
 */
inline void assert_throw_internal(bool p, const std::string &&msg)
{
    if (!p)
    {
        throw internal_error(msg);
    }
}

/*!
 * @brief Throw an internal_error if the parameter is null.
 */
template <class T>
T *check_null(T *t)
{
    if (t)
    {
        return t;
    }
    else
    {
        throw internal_error("unexpected null object");
    }
}

} // namespace erpcgen

#endif // _EMBEDDED_RPC__PARSEERRORS_H
