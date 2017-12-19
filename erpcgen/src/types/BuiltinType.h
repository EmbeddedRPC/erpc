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

#ifndef _EMBEDDED_RPC__BUILTINTYPE_H_
#define _EMBEDDED_RPC__BUILTINTYPE_H_

#include "DataType.h"
#include <string>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen {

/*!
 * @brief Represents the builtin atomic types.
 */
class BuiltinType : public DataType
{
public:
    /*!
     *  @brief Atomic builtin types.
     */
    enum _builtin_type
    {
        kBoolType,
        kInt8Type,
        kInt16Type,
        kInt32Type,
        kInt64Type,
        kUInt8Type,
        kUInt16Type,
        kUInt32Type,
        kUInt64Type,
        kFloatType,
        kDoubleType,
        kStringType,
        kBinaryType
    };

    /*!
     * @brief Constructor.
     *
     * This function set builtin type to given builtin type and name to given name.
     *
     * @param[in] name Given name.
     * @param[in] builtinType Given builtin type.
     */
    BuiltinType(const std::string &name, _builtin_type builtinType)
    : DataType(name, kBuiltinType)
    , m_builtinType(builtinType)
    {
    }

    /*!
     * @brief This function returns builtin type.
     *
     * @return Builtin type of current object.
     */
    _builtin_type getBuiltinType() const { return m_builtinType; }

    /*!
     * @brief This function return "true" value for identify builtin type.
     *
     * @retval true Always return true.
     */
    virtual bool isBuiltin() const { return true; }

    /*!
     * @brief This function return "true" value for identify scalar type.
     *
     * @retval true When builtin type is integer, float or boolean.
     * @retval false When builtin type isn't integer, float or boolean.
     */
    virtual bool isScalar() const { return (isInt() || isFloat() || isBool()) && !(isString() || isBinary()); }

    /*!
     * @brief This function return "true" value for identify int type.
     *
     * @retval true When builtin type is int.
     * @retval false When builtin type isn't int.
     */
    virtual bool isInt() const { return kInt8Type <= m_builtinType && m_builtinType <= kUInt64Type; }

    /*!
     * @brief This function return "true" value for identify float type.
     *
     * @retval true When builtin type is float.
     * @retval false When builtin type isn't float.
     */
    virtual bool isFloat() const { return m_builtinType == kFloatType || m_builtinType == kDoubleType; }

    /*!
     * @brief This function return "true" value for identify bool type.
     *
     * @retval true When builtin type is bool.
     * @retval false When builtin type isn't bool.
     */
    virtual bool isBool() const { return m_builtinType == kBoolType; }

    /*!
     * @brief This function return true/false value for identify string type.
     *
     * @retval true When builtin type is string.
     * @retval false When builtin type isn't string.
     */
    virtual bool isString() const { return m_builtinType == kStringType; }

    /*!
     * @brief This function return true/false value for identify binary type.
     *
     * @retval true When builtin type is binary.
     * @retval false When builtin type isn't binary.
     */
    virtual bool isBinary() const { return m_builtinType == kBinaryType; }

protected:
    _builtin_type m_builtinType; /*!< Builtin type of current object. */
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__BUILTINTYPE_H_
