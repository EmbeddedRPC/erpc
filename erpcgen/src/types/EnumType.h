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
    typedef std::vector<EnumMember *> member_vector_t; /*! Contains list of enum members */

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
