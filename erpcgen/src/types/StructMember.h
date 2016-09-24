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

#ifndef _EMBEDDED_RPC__STRUCTMEMBER_H_
#define _EMBEDDED_RPC__STRUCTMEMBER_H_

#include <string>
#include "DataType.h"
#include "Symbol.h"

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen
{
/*!
 *  @brief Supported directions types.
 */
enum _param_direction
{
    kInDirection,
    kOutDirection,
    kOutDirectionByref,
    kInoutDirection,
    kReturn,
    kNone
};

/*!
 * @brief Member of a struct.
 */
class StructMember : public Symbol
{
public:
    /*!
     * @brief Constructor.
     *
     * This function set name to given name and data type to given data type.
     *
     * @param[in] name Given name.
     * @param[in] dataType Given data type.
     */
    StructMember(const std::string &name, DataType *dataType)
    : Symbol(kStructMemberSymbol, name)
    , m_dataType(dataType)
    , m_paramDirection()
    , m_containList()
    , m_containString()
    {
    }

    /*!
     * @brief Constructor.
     *
     * This function set symbol token to given token and data type to given data type.
     *
     * @param[in] tok Given token.
     * @param[in] dataType Given data type.
     */
    StructMember(const Token &tok, DataType *dataType)
    : Symbol(kStructMemberSymbol, tok)
    , m_dataType(dataType)
    , m_paramDirection()
    , m_containList()
    , m_containString()
    {
    }

    /*!
     * @brief This function returns pointer to element data type.
     *
     * @return Pointer to element data type.
     */
    DataType *getDataType() { return m_dataType; }
    /*!
     * @brief This function will set element data type.
     *
     * @param[in] dataType Pointer to element data type.
     */
    void setDataType(DataType *dataType) { m_dataType = dataType; }
    /*!
     * @brief This function returns description about the struct member.
     *
     * @return String description about struct member.
     *
     * @see std::string AliasType::getDescription() const
     * @see std::string EnumType::getDescription() const
     * @see std::string EnumMember::getDescription() const
     * @see std::string StructType::getDescription() const
     * @see std::string VoidType::getDescription() const
     * @see std::string ArrayType::getDescription() const
     * @see std::string ListType::getDescription() const
     * @see std::string UnionType::getDescription() const
     */
    virtual std::string getDescription() const;

    /*!
     * @brief This function set direction type for parameter.
     *
     * This is used in function's parameters.They can be set as in, out, inout.
     *
     * @param[in] paramDirection Define direction type for parameter in functions.
     */
    void setDirection(_param_direction paramDirection) { m_paramDirection = paramDirection; }
    /*!
     * @brief This function returns routing for parameter.
     *
     * This is used in function's parameters.They can be set as in, out, inout.
     *
     * @return Returns routing type for parameter in functions.
     */
    _param_direction getDirection() { return m_paramDirection; }
    /*!
     * @brief This function set information about if member contains list data type.
     *
     * @param[in] containList Information about if member contains list data type.
     */
    void setContainList(bool containList) { m_containList = containList; }
    /*!
     * @brief This function set information about if member contains list data type.
     *
     * @return Information about if member contains list data type.
     */
    bool getContainList() { return m_containList; }
    /*!
     * @brief This function set information about if member contains string data type.
     *
     * @param[in] containString Information about if member contains string data type.
     */
    void setContainString(bool containString) { m_containString = containString; }
    /*!
     * @brief This function set information about if member contains string data type.
     *
     * @return Information about if member contains string data type.
     */
    bool getContainString() { return m_containString; }
protected:
    DataType *m_dataType;              /*!< Struct member data type. */
    _param_direction m_paramDirection; /*!< Direction in which is member used. */
    bool m_containList;                /*!< True when member contains list type */
    bool m_containString;              /*!< True when member contains string type */
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__STRUCTMEMBER_H_
