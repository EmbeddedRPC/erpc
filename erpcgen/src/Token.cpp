/*
 * The Clear BSD License
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 * that the following conditions are met:
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

#include "Token.h"
#include "ErpcLexer.h"
#include "ParseErrors.h"

using namespace erpcgen;
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

Token::Token(const Token &other)
: m_token(other.m_token)
, m_value()
, m_location(other.m_location)
{
    if (other.m_value)
    {
        m_value = other.m_value->clone();
    }
}

Token &Token::operator=(const Token &other)
{
    m_token = other.m_token;
    m_location = other.m_location;
    m_value.reset();

    if (other.m_value)
    {
        m_value = other.m_value->clone();
    }

    return *this;
}

const char *Token::getTokenName() const
{
    return get_token_name(m_token);
}

bool Token::isNumberTok() const
{
    return TOK_FLOAT_LITERAL == m_token || TOK_INT_LITERAL == m_token;
}

bool Token::isIdentifierTok() const
{
    return TOK_IDENT == m_token;
}

void Token::setLocation(const token_loc_t &first, const token_loc_t &last)
{
    m_location.m_firstLine = first.m_firstLine;
    m_location.m_lastLine = last.m_lastLine;
}

const string &Token::getStringValue() const
{
    const StringValue *s;
    if (m_value && (s = dynamic_cast<const StringValue *>(m_value.get())))
    {
        return s->getString();
    }
    else
    {
        throw internal_error(format_string("unexpected token type %s on line %d", this->getTokenName(),
                                           this->getLocation().m_firstLine));
    }
}

uint64_t Token::getIntValue() const
{
    const IntegerValue *s;
    if (m_value && (s = dynamic_cast<const IntegerValue *>(m_value.get())))
    {
        return s->getValue();
    }
    else
    {
        throw internal_error(format_string("unexpected token type %s on line %d", this->getTokenName(),
                                           this->getLocation().m_firstLine));
    }
}

bool Token::isBinaryOp() const
{
    if (string::npos != string("+-*/%&|^").find(static_cast<char>(m_token)))
    {
        return true;
    }
    if (TOK_RSHIFT == m_token || TOK_LSHIFT == m_token)
    {
        return true;
    }
    return false;
}
