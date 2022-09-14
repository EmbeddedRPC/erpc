/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__STRUCTTYPE_H_
#define _EMBEDDED_RPC__STRUCTTYPE_H_

#include "DataType.hpp"
#include "StructMember.hpp"
#include "SymbolScope.hpp"

#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen {

/*!
 * @brief Structure data type.
 */
class StructType : public DataType
{
public:
    typedef std::vector<StructMember *> member_vector_t; /*!< Vector of Struct members. */

    /*!
     * @brief Constructor.
     *
     * This function set name to given name.
     *
     * @param[in] name Given name.
     */
    explicit StructType(const std::string &name)
    : DataType(name, kStructType)
    {
    }

    /*!
     * @brief Constructor.
     *
     * This function set symbol token to given token.
     *
     * @param[in] tok Given token.
     */
    explicit StructType(const Token &tok)
    : DataType(tok, kStructType)
    {
    }

    /*!
     * @brief This function will add struct member to the struct.
     *
     * The function will add struct member given by pointer newMember to the struct members vector m_members.
     * Also this member will be added as symbol to struct symbol scope m_scope.
     *
     * @param[in] newMember Struct member pointer, which is added to struct members vector.
     */
    void addMember(StructMember *newMember);

    /*!
     * @brief This function returns symbol scope.
     *
     * @return Symbol scope.
     */
    SymbolScope &getScope() { return m_scope; }

    /*!
     * @brief This function returns struct members vector.
     *
     * @return Vector of struct members.
     */
    member_vector_t &getMembers() { return m_members; }

    /*!
     * @brief @brief This function returns description about the struct.
     *
     * @return String description about struct.
     *
     * @see std::string AliasType::getDescription() const
     * @see std::string EnumType::getDescription() const
     * @see std::string EnumMember::getDescription() const
     * @see std::string StructMember::getDescription() const
     * @see std::string VoidType::getDescription() const
     * @see std::string ArrayType::getDescription() const
     * @see std::string ListType::getDescription() const
     * @see std::string UnionType::getDescription() const
     */
    virtual std::string getDescription() const;

    /*!
     * @brief This function return true, when has member, which contains list type.
     *
     * @return True if structure contains list type.
     */
    bool containListMember();

    /*!
     * @brief This function return true, when has member, which contains string type.
     *
     * @return True if structure contains string type.
     */
    bool containStringMember();

    /*!
     * @brief This function return true, when has member, which contains flag byref.
     *
     * @return True if structure contains byref parameter.
     */
    bool containByrefMember();

protected:
    SymbolScope m_scope;       /*!< Symbol scope of current structure. */
    member_vector_t m_members; /*!< Vector of structure members. */
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__STRUCTTYPE_H_
