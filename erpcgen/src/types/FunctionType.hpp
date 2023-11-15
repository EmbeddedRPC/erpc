/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__FUNCTION_TYPE_H_
#define _EMBEDDED_RPC__FUNCTION_TYPE_H_

#include "DataType.hpp"
#include "Function.hpp"
#include "StructType.hpp"
#include "Symbol.hpp"

#include <string>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen {

/*!
 * @brief Function data type.
 *
 * The function's parameters are represented by a StructType object.
 */
class FunctionType : public FunctionBase, public DataType
{
public:
    typedef std::vector<Function *> c_function_list_t; /*!< List type of callback functions. */

    /*!
     * @brief Constructor.
     *
     * This function set symbol token to given token.
     *
     * @param[in] tok Given token.
     * @param[in] m_interface Parent interface.
     */
    FunctionType(const Token &tok, Interface *interface) :
    FunctionBase(interface), DataType(tok, data_type_t::kFunctionType), m_callbackFuns()
    {
    }

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
    virtual std::string getDescription() const override;

    /*!
     * @brief Returns list of callback functions.
     */
    c_function_list_t &getCallbackFuns() { return m_callbackFuns; }

protected:
    c_function_list_t m_callbackFuns; /*!< List of callback functions. */
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__FUNCTION_TYPE_H_
