/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
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
 * o Neither the name of the copyright holder nor the names of its
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
