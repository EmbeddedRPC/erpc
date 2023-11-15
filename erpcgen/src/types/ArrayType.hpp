/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__ARRAYTYPE_H_
#define _EMBEDDED_RPC__ARRAYTYPE_H_

#include "DataType.hpp"

#include <string>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen {

/*!
 * @brief Fixed length array of any other data type.
 */
class ArrayType : public DataType
{
public:
    /*!
     * @brief Constructor.
     *
     * This function set element type to given data type and element count to given count.
     *
     * @param[in] elementType Given data type.
     * @param[in] elementCount Given count.
     */
    ArrayType(DataType *elementType, uint32_t elementCount) :
    DataType("(array)", data_type_t::kArrayType), m_elementType(elementType), m_elementCount(elementCount)
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
     * @brief This function returns description about the array.
     *
     * @return String description about array.
     *
     * @see std::string AliasType::getDescription() const
     * @see std::string EnumType::getDescription() const
     * @see std::string EnumMember::getDescription() const
     * @see std::string StructMember::getDescription() const
     * @see std::string StructType::getDescription() const
     * @see std::string VoidType::getDescription() const
     * @see std::string ListType::getDescription() const
     * @see std::string UnionType::getDescription() const
     */
    virtual std::string getDescription() const override;

    /*!
     * @brief This function returns size of array.
     *
     * @return Size of array.
     */
    uint32_t &getElementCount() { return m_elementCount; }

protected:
    DataType *m_elementType; /*!< Array element data type. */
    uint32_t m_elementCount; /*!< Array size. */
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__ARRAYTYPE_H_
