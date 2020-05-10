/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__CONST_H_
#define _EMBEDDED_RPC__CONST_H_

#include "DataType.h"
#include "Symbol.h"
#include <string>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen {

/*!
 * @brief Const class
 */
class ConstType : public Symbol
{
public:
    /*!
     * @brief Constructor.
     *
     * This function set symbol token to given token, data type to given data type and value to given value.
     *
     * @param[in] tok Token, which contains name and location.
     * @param[in] dataType Given data type..
     * @param[in] value Given value.
     */
    ConstType(const Token &tok, DataType *dataType, Value *value)
    : Symbol(kConstSymbol, tok)
    , m_dataType(dataType)
    , m_value(value)
    {
    }

    /*!
     * @brief This function returns pointer to element data type.
     *
     * @return Pointer to element data type.
     */
    DataType *getDataType() { return m_dataType; }

    /*!
     * @brief This function returns value of constant data type.
     *
     * @return Pointer to value.
     */
    Value *getValue() { return m_value; }

    /*!
     * @brief This function sets constant data type.
     *
     * @param[in] dataType Constant data type.
     */
    void setDataType(DataType *dataType) { m_dataType = dataType; }

    /*!
     * @brief This function sets constant value.
     *
     * @param[in] value Constant value.
     */
    void setValue(Value *value) { m_value = value; }

protected:
    DataType *m_dataType; /*!< Constant data type. */
    Value *m_value;       /*!< Constant value. */
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__CONST_H_
