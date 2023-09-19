/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__INTERFACE_H_
#define _EMBEDDED_RPC__INTERFACE_H_

#include "Function.hpp"
#include "Symbol.hpp"
#include "SymbolScope.hpp"

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
    typedef std::vector<Function *> function_vector_t; /*!< Vector of Interface functions. */

    /*!
     * @brief Constructor.
     *
     * This function set symbol token to given token.
     *
     * @param[in] tok Given token.
     */
    explicit Interface(const Token &tok)
    : Symbol(symbol_type_t::kInterfaceSymbol, tok)
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
    virtual std::string getDescription() const override;

protected:
    SymbolScope m_scope;           /*!< Scope which interface belongs to. */
    function_vector_t m_functions; /*!< Vector of interface functions. */
    uint32_t m_uniqueId;           /*!< Interface unique id. */

    static uint32_t s_idCounter; /*!< Interface id counter. Each interface will increase this. */
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__INTERFACE_H_
