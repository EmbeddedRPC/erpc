/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
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

#ifndef _EMBEDDED_RPC__LISTTYPE_H_
#define _EMBEDDED_RPC__LISTTYPE_H_

#include <string>
#include "DataType.h"

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen
{
/*!
 * @brief Variable length list of another data type.
 */
class ListType : public DataType
{
public:
    /*!
     * @brief Constructor.
     *
     * This function set element type to given data type.
     *
     * @param[in] elementType Given data type.
     */
    ListType(DataType *elementType)
    : DataType("(list)", kListType)
    , m_elementType(elementType)
    , m_lengthVariableName("")
    {
    }

    /*!
     * @brief This function returns element data type.
     *
     * @return Element data type.
     */
    DataType *getElementType() const { return m_elementType; }
    /*!
     * @brief This function set element data type.
     *
     * @param[in] elementType Element data type.
     */
    void setElementType(DataType *elementType) { m_elementType = elementType; }
    /*!
     * @brief This function return "true" value for identify list type.
     *
     * @retval true Always return true.
     */
    virtual bool isList() const { return true; }
    /*!
     * @brief This function returns description about the list.
     *
     * @return String description about list.
     *
     * @see std::string AliasType::getDescription() const
     * @see std::string EnumType::getDescription() const
     * @see std::string EnumMember::getDescription() const
     * @see std::string StructMember::getDescription() const
     * @see std::string StructType::getDescription() const
     * @see std::string VoidType::getDescription() const
     * @see std::string ArrayType::getDescription() const
     * @see std::string UnionType::getDescription() const
     */
    virtual std::string getDescription() const;

    /*!
     * @brief This function set variable name. This variable is used in generated file to store size of list.
     *
     * @param[in] lengthVariableName Given name for variable.
     */
    void setLengthVariableName(const std::string &lengthVariableName) { m_lengthVariableName = lengthVariableName; }
    /*!
     * @brief This function returns variable name. This variable is used in generated file to store size of list.
     *
     * @return Return name for variable.
     */
    std::string &getLengthVariableName() { return m_lengthVariableName; }
    /*!
     * @brief This function returns information if object contains length variable.
     *
     * @retval true When list contains length variable.
     * @retval false When list doesn't contain length variable.
     */
    bool hasLengthVariable() { return "" != m_lengthVariableName; }
protected:
    DataType *m_elementType;          /*!< List element data type. */
    std::string m_lengthVariableName; /*!< List length variable name. */
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__LISTTYPE_H_
