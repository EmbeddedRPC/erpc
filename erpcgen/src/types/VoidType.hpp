/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__VOIDTYPE_H_
#define _EMBEDDED_RPC__VOIDTYPE_H_

#include "DataType.hpp"

#include <string>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen {

/*!
 * @brief Void data type.
 */
class VoidType : public DataType
{
public:
    /*!
     * @brief Constructor.
     *
     * This function set name to "(void)".
     */
    VoidType()
    : DataType("(void)", kVoidType)
    {
    }

    /*!
     * @brief This function returns description about the void.
     *
     * @retval "<void>" Always return "<void>" as string.
     *
     * @see std::string AliasType::getDescription() const
     * @see std::string EnumType::getDescription() const
     * @see std::string EnumMember::getDescription() const
     * @see std::string StructMember::getDescription() const
     * @see std::string StructType::getDescription() const
     * @see std::string ArrayType::getDescription() const
     * @see std::string ListType::getDescription() const
     */
    virtual std::string getDescription() const override { return "<void>"; }
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__VOIDTYPE_H_
