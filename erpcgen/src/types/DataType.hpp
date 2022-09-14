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

#include "Symbol.hpp"

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
    enum data_type_t
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
    explicit DataType(data_type_t dataType)
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
    DataType(const std::string &name, data_type_t dataType)
    : Symbol(kTypenameSymbol, name)
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
    DataType(const Token &tok, data_type_t dataType)
    : Symbol(kTypenameSymbol, tok)
    , m_dataType(dataType)
    {
    }

    /*!
     * @brief This function returns data type.
     *
     * @return Data type of current object.
     */
    data_type_t getDataType() const { return m_dataType; }

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
     * @brief This function is testing data type.
     *
     * @retval true When data type is AliasType.
     * @retval false When data type isn't AliasType.
     */
    bool isAlias() const { return (m_dataType == kAliasType); }

    /*!
     * @brief This function is testing data type.
     *
     * @retval true When data type is ArrayType.
     * @retval false When data type isn't ArrayType.
     */
    bool isArray() const { return (m_dataType == kArrayType); }

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
     * @brief This function is testing data type.
     *
     * @retval true When data type is BuiltinType.
     * @retval false When data type isn't BuiltinType.
     */
    bool isBuiltin() const { return (m_dataType == kBuiltinType); }

    /*!
     * @brief This function is testing data type.
     *
     * @retval true When data type is EnumTyp.
     * @retval false When data type isn't EnumTyp.
     */
    bool isEnum() const { return (m_dataType == kEnumType); }

    /*!
     * @brief This function is testing data type.
     *
     * @retval true When data type is FunctionType.
     * @retval false When data type isn't FunctionType.
     */
    bool isFunction() const { return (m_dataType == kFunctionType); }

    /*!
     * @brief This function is testing data type.
     *
     * @retval true When data type is ListType.
     * @retval false When data type isn't ListType.
     */
    bool isList() const { return (m_dataType == kListType); }

    /*!
     * @brief This function return "false" value as default for identify scalar builtin type.
     *
     * @retval false Always return false;
     */
    virtual bool isScalar() const { return false; }

    /*!
     * @brief This function return "true" value for identify int type.
     *
     * @retval false Always return false;
     */
    virtual bool isInt() const { return false; }

    /*!
     * @brief This function return "true" value for identify float type.
     *
     * @retval false Always return false;
     */
    virtual bool isFloat() const { return false; }

    /*!
     * @brief This function return "false" value as default for identify string type.
     *
     * @retval false Always return false.
     */
    virtual bool isString() const { return false; }

    /*!
     * @brief This function return "false" value as default for identify ustring type.
     *
     * @retval false Always return false.
     */
    virtual bool isUString() const { return false; }

    /*!
     * @brief This function is testing data type.
     *
     * @retval true When data type is StructType.
     * @retval false When data type isn't StructType.
     */
    bool isStruct() const { return (m_dataType == kStructType); }

    /*!
     * @brief This function is testing data type.
     *
     * @retval true When data type is UnionType.
     * @retval false When data type isn't UnionType.
     */
    bool isUnion() const { return (m_dataType == kUnionType); }

    /*!
     * @brief This function is testing data type.
     *
     * @retval true When data type is VoidType.
     * @retval false When data type isn't VoidType.
     */
    bool isVoid() const { return (m_dataType == kVoidType); }

protected:
    data_type_t m_dataType; /*!< Data type of current object */
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__DATATYPE_H_
