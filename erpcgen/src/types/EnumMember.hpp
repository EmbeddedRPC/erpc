/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__ENUMMEBER_H_
#define _EMBEDDED_RPC__ENUMMEBER_H_

#include "Symbol.hpp"

#include <string>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen {

/*!
 * @brief Member of a enum.
 */
class EnumMember : public Symbol
{
public:
    /*!
     * @brief Constructor.
     *
     * This function sets the value and token
     *
     * @param[in] tok Token, which contains name and location.
     * @param[in] value Given value.
     */
    EnumMember(const Token &tok, uint32_t value)
    : Symbol(symbol_type_t::kEnumMemberSymbol, tok)
    , m_value(value)
    , m_valueSet(true)
    {
    }

    /*!
     * @brief Constructor.
     *
     * This function set token to given token.
     *
     * @param[in] tok Token, which contains name and location.
     */
    explicit EnumMember(const Token &tok)
    : Symbol(symbol_type_t::kEnumMemberSymbol, tok)
    , m_value(-1)
    , m_valueSet(false)
    {
    }

    /*!
     * @brief This function set enum member value.
     *
     * @param[in] value Value of enum member.
     */
    void setValue(IntegerValue value)
    {
        m_value = value;
        m_valueSet = true;
    }

    /*!
     * @brief This function returns enum member value.
     *
     * @return String representation of enum member value.
     */
    uint32_t getValue() const { return (uint32_t)m_value; }

    /*!
     * @brief This function return true if enum member has set value.
     *
     * @retval true When enum member has set value.
     * @retval false When enum member has not set value.
     */
    bool hasValue() const { return m_valueSet; }

    /*!
     * @brief This function returns description about the enum member.
     *
     * @return String description about enum member.
     *
     * @see std::string AliasType::getDescription() const
     * @see std::string EnumType::getDescription() const
     * @see std::string StructMember::getDescription() const
     * @see std::string StructType::getDescription() const
     * @see std::string VoidType::getDescription() const
     * @see std::string ArrayType::getDescription() const
     * @see std::string ListType::getDescription() const
     * @see std::string UnionType::getDescription() const
     */
    virtual std::string getDescription() const override;

protected:
    IntegerValue m_value; /*!< Integer value of enum member. */
    bool m_valueSet;
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__ENUMMEBER_H_
