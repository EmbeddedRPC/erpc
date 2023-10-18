/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__ALIASTYPE_H_
#define _EMBEDDED_RPC__ALIASTYPE_H_

#include "DataType.hpp"

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
    AliasType(const std::string &name, DataType *elementType) : DataType(name, kAliasType), m_elementType(elementType)
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
    AliasType(const Token &tok, DataType *elementType) : DataType(tok, kAliasType), m_elementType(elementType) {}

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
    virtual std::string getDescription() const override;

protected:
    DataType *m_elementType; /*!< Alias element data type. */
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__ALIASTYPE_H_
