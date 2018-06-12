/*
 * The Clear BSD License
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 *  that the following conditions are met:
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
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
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
