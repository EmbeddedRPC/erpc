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

#include "DataType.hpp"

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
    enum class builtin_type_t
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
    BuiltinType(const std::string &name, builtin_type_t builtinType) :
    DataType(name, data_type_t::kBuiltinType), m_builtinType(builtinType)
    {
    }

    /*!
     * @brief This function returns builtin type.
     *
     * @return Builtin type of current object.
     */
    builtin_type_t getBuiltinType() const { return m_builtinType; }

    /*!
     * @brief This function return "true" value for identify scalar type.
     *
     * @retval true When builtin type is integer, float or boolean.
     * @retval false When builtin type isn't integer, float or boolean.
     */
    virtual bool isScalar() const override { return (isInt() || isFloat() || isBool()) && !(isString() || isBinary()); }

    /*!
     * @brief This function return "true" value for identify int type.
     *
     * @retval true When builtin type is int.
     * @retval false When builtin type isn't int.
     */
    virtual bool isInt() const override
    {
        return ((builtin_type_t::kInt8Type <= m_builtinType) && (m_builtinType <= builtin_type_t::kUInt64Type));
    }

    /*!
     * @brief This function return "true" value for identify float type.
     *
     * @retval true When builtin type is float.
     * @retval false When builtin type isn't float.
     */
    virtual bool isFloat() const override
    {
        return ((m_builtinType == builtin_type_t::kFloatType) || (m_builtinType == builtin_type_t::kDoubleType));
    }

    /*!
     * @brief This function return "true" value for identify bool type.
     *
     * @retval true When builtin type is bool.
     * @retval false When builtin type isn't bool.
     */
    virtual bool isBool() const override { return m_builtinType == builtin_type_t::kBoolType; }

    /*!
     * @brief This function return true/false value for identify string type.
     *
     * @retval true When builtin type is string or ustring.
     * @retval false When builtin type isn't string or ustring.
     */
    virtual bool isString() const override
    {
        return ((m_builtinType == builtin_type_t::kStringType) || (m_builtinType == builtin_type_t::kUStringType));
    }

    /*!
     * @brief This function return true/false value for identify ustring type.
     *
     * @retval true When builtin type is ustring.
     * @retval false When builtin type isn't ustring.
     */
    virtual bool isUString() const override { return m_builtinType == builtin_type_t::kUStringType; }

    /*!
     * @brief This function return true/false value for identify binary type.
     *
     * @retval true When builtin type is binary.
     * @retval false When builtin type isn't binary.
     */
    virtual bool isBinary() const override { return m_builtinType == builtin_type_t::kBinaryType; }

protected:
    builtin_type_t m_builtinType; /*!< Builtin type of current object. */
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__BUILTINTYPE_H_
