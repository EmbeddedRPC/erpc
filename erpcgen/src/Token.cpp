/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "Token.hpp"

#include "ErpcLexer.hpp"
#include "ParseErrors.hpp"

using namespace erpcgen;
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

Token::Token(const Token &other) : m_token(other.m_token), m_value(), m_location(other.m_location)
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
