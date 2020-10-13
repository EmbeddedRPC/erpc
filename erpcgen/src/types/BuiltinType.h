/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
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
        kUStringType,
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
     * @retval true When builtin type is string or ustring.
     * @retval false When builtin type isn't string or ustring.
     */
    virtual bool isString() const { return m_builtinType == kStringType || m_builtinType == kUStringType; }

    /*!
     * @brief This function return true/false value for identify ustring type.
     *
     * @retval true When builtin type is ustring.
     * @retval false When builtin type isn't ustring.
     */
    virtual bool isUString() const { return m_builtinType == kUStringType; }

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
