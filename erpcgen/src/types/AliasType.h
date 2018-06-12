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

#ifndef _EMBEDDED_RPC__ALIASTYPE_H_
#define _EMBEDDED_RPC__ALIASTYPE_H_

#include "DataType.h"
#include <string>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen {

/*!
 * @brief Typedef.
 */
class AliasType : public DataType
{
public:
    /*!
     * @brief Constructor.
     *
     * This function set element type to given data type and set DataType name to given name.
     *
     * @param[in] name Name.
     * @param[in] elementType Given data type.
     */
    AliasType(std::string name, DataType *elementType)
    : DataType(name, kAliasType, kAliasTypeSymbol)
    , m_elementType(elementType)
    {
    }

    /*!
     * @brief Constructor.
     *
     * This function set element type to given data type and set DataType token to given token.
     *
     * @param[in] tok Token, which contains name and location.
     * @param[in] elementType Given data type.
     */
    AliasType(const Token &tok, DataType *elementType)
    : DataType(tok, kAliasType, kAliasTypeSymbol)
    , m_elementType(elementType)
    {
    }

    /*!
     * @brief This function return "true" value for identify alias type.
     *
     * @retval true Always return true.
     */
    virtual bool isAlias() const { return true; }

    /*!
     * @brief This function return element data type.
     *
     * @return Element data type.
     */
    DataType *getElementType() const { return m_elementType; }

    /*!
     * @brief This function set element data type.
     *
     * This function is used when enum or struct anonymous data type is created.
     *
     * @param[in] elementType Element data type.
     */
    void setElementType(DataType *elementType) { m_elementType = elementType; }

    /*!
     * @brief This function returns description about the alias.
     *
     * @return String description about alias.
     *
     * @see std::string EnumMember::getDescription() const
     * @see std::string EnumType::getDescription() const
     * @see std::string StructMember::getDescription() const
     * @see std::string StructType::getDescription() const
     * @see std::string VoidType::getDescription() const
     * @see std::string ArrayType::getDescription() const
     * @see std::string ListType::getDescription() const
     * @see std::string UnionType::getDescription() const
     */
    virtual std::string getDescription() const;

protected:
    DataType *m_elementType; /*!< Alias element data type. */
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__ALIASTYPE_H_
