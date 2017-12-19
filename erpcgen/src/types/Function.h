/*
 * The Clear BSD License
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 * that the following conditions are met:
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

#ifndef _EMBEDDED_RPC__FUNCTION_H_
#define _EMBEDDED_RPC__FUNCTION_H_

#include "DataType.h"
#include "StructType.h"
#include "Symbol.h"
#include <string>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen {

/*!
 * @brief Function base declaration.
 *
 * The function's parameters are represented by a StructType object.
 */
class FunctionBase
{
public:
    /*!
     * @brief Constructor.
     */
    FunctionBase()
    : m_parameters("(fn)")
    , m_returnType(nullptr)
    , m_isOneway(false)
    {
    }

    virtual ~FunctionBase(){};

    /*!
     * @brief This function returns function parameters.
     *
     * @return Function parameters.
     */
    StructType &getParameters() { return m_parameters; }

    /*!
     * @brief This function returns data type of function return value.
     *
     * @return Return data type of function return value.
     */
    DataType *getReturnType() { return m_returnType->getDataType(); }

    /*!
     * @brief This function returns data type of function return value.
     *
     * @return Return data type of function return value.
     */
    StructMember *getReturnStructMemberType() { return m_returnType; }

    /*!
     * @brief This function set data type of function return value.
     *
     * @param[in] returnType Function return value data type.
     */
    void setReturnStructMemberType(StructMember *returnType) { m_returnType = returnType; }

    /*!
     * @brief This function returns true/false, when function return type is/isn't oneway.
     *
     * @retval true Function return type is oneway.
     * @retval false Function return type isn't oneway.
     */
    bool isOneway() const { return m_isOneway; }

    /*!
     * @brief This function set true/false, when function return type is/isn't oneway.
     *
     * @param[in] isOneway Set, if function return type is oneway.
     */
    void setIsOneway(bool isOneway) { m_isOneway = isOneway; }

    /*!
     * @brief This function returns description about the interface function.
     *
     * @return String description about interface function.
     *
     * @see std::string AliasType::getDescription() const
     * @see std::string EnumMember::getDescription() const
     * @see std::string EnumType::getDescription() const
     * @see std::string StructMember::getDescription() const
     * @see std::string StructType::getDescription() const
     * @see std::string VoidType::getDescription() const
     * @see std::string ArrayType::getDescription() const
     * @see std::string ListType::getDescription() const
     */
    virtual std::string getDescription() const = 0;

protected:
    StructType m_parameters;    /*!< Function parameters are saved as structure members. */
    StructMember *m_returnType; /*!< Function return data type. */
    bool m_isOneway;            /*!< If false then communication is bidirectional. */
};

class Interface;
class FunctionType;
/*!
 * @brief Function declaration.
 *
 * The function's parameters are represented by a StructType object.
 */
class Function : public FunctionBase, public Symbol
{
public:
    /*!
     * @brief Constructor.
     *
     * This function set symbol token to given token.
     *
     * @param[in] tok Given token.
     * @param[in] m_interface Parent interface.
     */
    Function(const Token &tok, Interface *interface)
    : Symbol(kFunctionSymbol, tok)
    , FunctionBase()
    , m_uniqueId(++s_idCounter)
    , m_interface(interface)
    , m_functionType(nullptr)
    {
    }

    /*!
     * @brief Constructor.
     *
     * This function set symbol token to given token, uniqueId and idCounter to given uniqueId.
     *
     * @param[in] tok Given token.
     * @param[in] m_interface Parent interface.
     * @param[in] uniqueId Given unique function id.
     */
    Function(const Token &tok, Interface *interface, uint32_t uniqueId)
    : Symbol(kFunctionSymbol, tok)
    , FunctionBase()
    , m_uniqueId(uniqueId)
    , m_interface(interface)
    , m_functionType(nullptr)
    {
        s_idCounter = uniqueId;
    }

    /*!
     * @brief This function returns function unique id.
     *
     * @return Function unique id.
     */
    uint32_t getUniqueId() const { return m_uniqueId; }

    /*!
     * @brief This function set function unique id.
     *
     * @param[in] newId New function unique id.
     */
    void setUniqueId(uint32_t newId) { m_uniqueId = newId; }

    /*!
     * @brief This function returns parent Interface.
     *
     * @return parent Interface.
     */
    Interface *getInterface() const { return m_interface; }

    /*!
     * @brief This function returns description about the interface function.
     *
     * @return String description about interface function.
     *
     * @see std::string AliasType::getDescription() const
     * @see std::string EnumMember::getDescription() const
     * @see std::string EnumType::getDescription() const
     * @see std::string StructMember::getDescription() const
     * @see std::string StructType::getDescription() const
     * @see std::string VoidType::getDescription() const
     * @see std::string ArrayType::getDescription() const
     * @see std::string ListType::getDescription() const
     */
    virtual std::string getDescription() const;

    /*!
     * @brief This function sets FunctionType (callback type).
     *
     * @param[in] functionType FunctionType (callback type).
     */
    void setFunctionType(FunctionType *functionType) { m_functionType = functionType; }

    /*!
     * @brief This function returns FunctionType (callback type).
     *
     * @return FunctionType (callback type).
     */
    FunctionType *getFunctionType() const { return m_functionType; }

protected:
    uint32_t m_uniqueId;          /*!< Function unique id. */
    Interface *m_interface;       /*!< Parent interface. */
    FunctionType *m_functionType; /*!< Parent interface. */

    static uint32_t s_idCounter; /*!< Function id counter. Each function will increase this. */
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__FUNCTION_H_
