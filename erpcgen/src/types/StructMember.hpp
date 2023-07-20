/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__STRUCTMEMBER_H_
#define _EMBEDDED_RPC__STRUCTMEMBER_H_

#include "DataType.hpp"
#include "Symbol.hpp"

#include <string>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen {

/*!
 *  @brief Supported directions types.
 */
enum _param_direction
{
    kInDirection,
    kOutDirection,
    kInoutDirection,
    kReturn
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
     * @param[in] ifaceScope Scope name.
     */
    StructMember(const std::string &name, DataType *dataType, const std::string &ifaceScope = "")
    : Symbol(kStructMemberSymbol, name)
    , m_dataType(dataType)
    , m_paramDirection()
    , m_containList()
    , m_containString()
    , m_byref(false)
    , m_ifaceScope(ifaceScope)
    {
    }

    /*!
     * @brief Constructor.
     *
     * This function set symbol token to given token and data type to given data type.
     *
     * @param[in] tok Given token.
     * @param[in] dataType Given data type.
     * @param[in] ifaceScope Scope name.
     */
    StructMember(const Token &tok, DataType *dataType, const std::string &ifaceScope = "")
    : Symbol(kStructMemberSymbol, tok)
    , m_dataType(dataType)
    , m_paramDirection()
    , m_containList()
    , m_containString()
    , m_byref(false)
    , m_ifaceScope(ifaceScope)
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
    virtual std::string getDescription() const override;

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
    _param_direction getDirection() const { return m_paramDirection; }

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

    /*!
     * @brief This function returns information if parameter is byref.
     *
     * @retval True when paramter is byref, else false.
     */
    bool isByref() { return m_byref; }

    /*!
     * @brief This function sets byref flag to the paramter.
     *
     * @param[in] byref Parameter flag to set byref option.
     */
    void setByref(bool byref) { m_byref = byref; }

    std::string getIfaceScope() { return m_ifaceScope; }

protected:
    DataType *m_dataType;              /*!< Struct member data type. */
    _param_direction m_paramDirection; /*!< Direction in which is member used. */
    bool m_containList;                /*!< True when member contains list type */
    bool m_containString;              /*!< True when member contains string type */
    bool m_byref;                      /*!< True when member is byref type */
    std::string m_ifaceScope;          /*!< In case of callback type interface scope can be set */
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__STRUCTMEMBER_H_
