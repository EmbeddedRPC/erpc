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

#ifndef _EMBEDDED_RPC__INTERFACE_H_
#define _EMBEDDED_RPC__INTERFACE_H_

#include "Function.h"
#include "Symbol.h"
#include "SymbolScope.h"
#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen {

/*!
 * @brief An interface that contains functions.
 */
class Interface : public Symbol
{
public:
    typedef std::vector<Function *> function_vector_t;

    /*!
     * @brief Constructor.
     *
     * This function set symbol token to given token.
     *
     * @param[in] tok Given token.
     */
    Interface(const Token &tok)
    : Symbol(kInterfaceSymbol, tok)
    , m_scope()
    , m_uniqueId(s_idCounter++)
    {
    }

    /*!
     * @brief This function will add function to the interface.
     *
     * The function will add function given by pointer func to the interface members vector m_functions.
     * Also this member will be added as symbol to interface symbol scope m_scope.
     *
     * @param[in] func Function pointer, which is added to interface members vector.
     */
    void addFunction(Function *func);

    /*!
     * @brief This function return symbol scope.
     *
     * @return Symbol scope.
     */
    SymbolScope &getScope() { return m_scope; }

    /*!
     * @brief This function return interface functions vector.
     *
     * @return Interface functions vector.
     */
    function_vector_t &getFunctions() { return m_functions; }

    /*!
     * @brief This function get unique id of interface.
     *
     * @return Unique id for interface.
     */
    uint32_t getUniqueId() const { return m_uniqueId; }

    /*!
     * @brief This function set unique id for interface.
     *
     * @param[in] newId Unique id for interface.
     */
    void setUniqueId(uint32_t newId) { m_uniqueId = newId; }

    /*!
     * @brief This function returns description about the interface.
     *
     * @return String description about interface.
     *
     * @see std::string Function::getDescription() const
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

protected:
    SymbolScope m_scope;           /*!< Scope which interface belongs to. */
    function_vector_t m_functions; /*!< Vector of interface functions. */
    uint32_t m_uniqueId;           /*!< Interface unique id. */

    static uint32_t s_idCounter; /*!< Interface id counter. Each interface will increase this. */
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__INTERFACE_H_
