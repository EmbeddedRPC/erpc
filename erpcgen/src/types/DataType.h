/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__DATATYPE_H_
#define _EMBEDDED_RPC__DATATYPE_H_

#include "Symbol.h"
#include <string>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen {

/*!
 * @brief Base class for data types.
 */
class DataType : public Symbol
{
public:
    /*!
     *  @brief Supported data types.
     */
    enum _data_type
    {
        kAliasType,
        kArrayType,
        kBuiltinType,
        kEnumType,
        kFunctionType,
        kListType,
        kStructType,
        kUnionType,
        kVoidType
    };

    /*!
     * @brief Constructor.
     *
     * This function set data type to given data type.
     *
     * @param[in] dataType Given data type.
     */
    DataType(_data_type dataType)
    : Symbol(kTypenameSymbol)
    , m_dataType(dataType)
    {
    }

    /*!
     * @brief Constructor.
     *
     * This function set data type to given data type and symbol name to given name.
     *
     * @param[in] dataType Given data type.
     * @param[in] name Given name for symbol.
     */
    DataType(const std::string &name, _data_type dataType)
    : Symbol(kTypenameSymbol, name)
    , m_dataType(dataType)
    {
    }

    /*!
     * @brief Constructor.
     *
     * This function set data type to given data type, symbol name to given name and symbol type to given symbol type.
     *
     * @param[in] name Given name for symbol.
     * @param[in] dataType Given data type.
     * @param[in] symbolType Given symbol type for symbol.
     */
    DataType(const std::string &name, _data_type dataType, symbol_type_t symbolType)
    : Symbol(symbolType, name)
    , m_dataType(dataType)
    {
    }

    /*!
     * @brief Constructor.
     *
     * This function set data type to given data type, symbol token to given token and symbol type to given symbol type.
     *
     * @param[in] tok Token, which contains name and location.
     * @param[in] dataType Given data type.
     * @param[in] symbolType Given symbol type for symbol.
     */
    DataType(const Token &tok, _data_type dataType, symbol_type_t symbolType)
    : Symbol(symbolType, tok)
    , m_dataType(dataType)
    {
    }

    /*!
     * @brief This function returns data type.
     *
     * @return Data type of current object.
     */
    _data_type getDataType() const { return m_dataType; }

    /*!
     * @brief This function returns pointer to true data type instead of alias.
     *
     * The function returns pointer to true data type instead of alias name.
     *
     * @return True data type of current object.
     */
    DataType *getTrueDataType();

    /*! @brief This function returns pointer to true data type (enum, builtin, structs) except lists and arrays.
     *
     * The function returns pointer to true data type (enum, builtin, structs) except lists and arrays.
     * When array or list data type are founded, it will search in their elements data types for last element data type
     * which will be (enum, builtin, structs).
     *
     * @return Pointer to true data type (enum, builtin, structs) of current object.
     */
    DataType *getTrueContainerDataType();

    /*!
     * @brief This function return "false" value as default for identify alias type.
     *
     * @retval false Always return false.
     */
    virtual bool isAlias() const { return false; }

    /*!
     * @brief This function return "false" value as default for identify array type.
     *
     * @retval false Always return false.
     */
    virtual bool isArray() const { return false; }

    /*!
     * @brief This function return "false" value as default for identify binary type.
     *
     * @retval false Always return false.
     */
    virtual bool isBinary() const { return false; }

    /*!
     * @brief This function return "false" value as default for identify bool type.
     *
     * @retval false Always return false.
     */
    virtual bool isBool() const { return false; }

    /*!
     * @brief This function return "false" value as default for identify builtin type.
     *
     * @retval false Always return false.
     */
    virtual bool isBuiltin() const { return false; }

    /*!
     * @brief This function return "false" value as default for identify enum type.
     *
     * @retval false Always return false.
     */
    virtual bool isEnum() const { return false; }

    /*!
     * @brief This function return "false" value as default for identify function type.
     *
     * @retval false Always return false.
     */
    virtual bool isFunction() const { return false; }

    /*!
     * @brief This function return "false" value as default for identify list type.
     *
     * @retval false Always return false.
     */
    virtual bool isList() const { return false; }

    /*!
     * @brief This function return "false" value as default for identify scalar builtin type.
     *
     * @retval false Always return false;
     */
    virtual bool isScalar() const { return false; }

    /*!
     * @brief This function return "false" value as default for identify string type.
     *
     * @retval false Always return false.
     */
    virtual bool isString() const { return false; }

    /*!
     * @brief This function return "false" value as default for identify struct type.
     *
     * @retval false Always return false.
     */
    virtual bool isStruct() const { return false; }

    /*!
     * @brief This function return "false" value as default for identify union type.
     *
     * @retval false Always return false.
     */
    virtual bool isUnion() const { return false; }

    /*!
     * @brief This function return "false" value as default for identify void type.
     *
     * @retval false Always return false.
     */
    virtual bool isVoid() const { return false; }

protected:
    _data_type m_dataType; /*!< Data type of current object */
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__DATATYPE_H_
