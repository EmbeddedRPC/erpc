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

#ifndef _EMBEDDED_RPC__STRUCTTYPE_H_
#define _EMBEDDED_RPC__STRUCTTYPE_H_

#include "DataType.h"
#include "StructMember.h"
#include "SymbolScope.h"
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
    typedef std::vector<StructMember *> member_vector_t;

    /*!
     * @brief Constructor.
     *
     * This function set name to given name.
     *
     * @param[in] name Given name.
     */
    StructType(const std::string &name)
    : DataType(name, kStructType, kStructTypeSymbol)
    {
    }

    /*!
     * @brief Constructor.
     *
     * This function set symbol token to given token.
     *
     * @param[in] tok Given token.
     */
    StructType(const Token &tok)
    : DataType(tok, kStructType, kStructTypeSymbol)
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
     * @brief This function return "true" value for identify struct type.
     *
     * @retval true Always return true.
     */
    virtual bool isStruct() const { return true; }

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
