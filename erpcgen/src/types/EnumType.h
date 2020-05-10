/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__ENUMTYPE_H_
#define _EMBEDDED_RPC__ENUMTYPE_H_

#include "DataType.h"
#include "EnumMember.h"
#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen {

/*!
 * @brief Enumerate data type.
 */
class EnumType : public DataType
{
public:
    typedef std::vector<EnumMember *> member_vector_t; /*!< Contains list of enum members */

    /*!
     * @brief Constructor.
     *
     * This function set token to given token.
     *
     * @param[in] tok Given token.
     */
    EnumType(const Token &tok)
    : DataType(tok, kEnumType, kEnumTypeSymbol)
    {
    }

    /*!
     * @brief Constructor.
     *
     * This function set DataType with default name.
     */
    EnumType()
    : DataType("", kEnumType, kEnumTypeSymbol)
    {
    }

    /*!
     * @brief This function will add enum member to the enum.
     *
     * The function will add enum member given by pointer newMember to the enum members vector m_members.
     *
     * @param[in] newMember Enum member pointer, which is added to enum members vector.
     */
    void addMember(EnumMember *newMember);

    /*!
     * @brief This function returns vector of enum members.
     *
     * @return Enum members vector.
     */
    member_vector_t &getMembers() { return m_members; }

    /*!
     * @brief This function returns enum member based on given name.
     *
     * @return Enum member.
     */
    EnumMember *getMember(std::string name);

    /*!
     * @brief This function return "true" value for identify enum type.
     *
     * @retval true Always return true.
     */
    virtual bool isEnum() const { return true; }

    /*!
     * @brief This function returns the next value to use for an enum member
     *
     * @retval uint32_t next value for enum
     */
    uint32_t getNextValue() { return m_currentValue++; }

    /*!
     * @brief This function set current value.
     *
     * @param[in] value Current value.
     */
    void setCurrentValue(uint32_t value) { m_currentValue = value; }

    /*!
     * @brief This function returns description about the enum.
     *
     * @return String description about enum.
     *
     * @see std::string AliasType::getDescription() const
     * @see std::string EnumMember::getDescription() const
     * @see std::string StructMember::getDescription() const
     * @see std::string StructType::getDescription() const
     * @see std::string VoidType::getDescription() const
     * @see std::string ArrayType::getDescription() const
     * @see std::string ListType::getDescription() const
     * @see std::string UnionType::getDescription() const
     */
    virtual std::string getDescription() const;

protected:
    member_vector_t m_members;   /*!< Vector of enum members. */
    uint32_t m_currentValue = 0; /*!< Value for enum member. */
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__ENUMTYPE_H_
