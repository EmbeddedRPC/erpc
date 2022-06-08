/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__UNIQUEIDCHECKER_H_
#define _EMBEDDED_RPC__UNIQUEIDCHECKER_H_
#include "Interface.hpp"
#include "InterfaceDefinition.hpp"
#include "SymbolScope.hpp"

#include <cstdio>
#include <set>
#include <string>
#include <utility>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen {

/*!
 * @brief Add ids to interfaces and functions.
 */
class UniqueIdChecker
{
public:
    typedef std::pair<int, std::string> idAndName_t; /*!< Pair id of function/interface with name. */

    /*!
     * @brief This function will set unique id numbers for functions and interfaces
     *
     * @param[in] def Interface definition variable.
     */
    void makeIdsUnique(erpcgen::InterfaceDefinition &def);

private:
    /*!
     * @brief This function will go through interfaces and set initial value of id number.
     *
     * @param[in] ifaces Vector of interfaces.
     */
    void initUsedInterfaceIds(erpcgen::SymbolScope::symbol_vector_t ifaces);

    /*!
     * @brief This function will go through interface functions and set initial value of id number.
     *
     * @param[in] iface Interface.
     */
    void initUsedFunctionIds(erpcgen::Interface *iface);

    /*!
     * @brief This helper function sets a function symbol's unique id from the specified annotation.
     *
     * @param[inout] fn Function whose id will be modified.
     * @param[in] idAnnotation Id value that will be assigned to fn.
     */
    void setFunctionId(Function *fn, Annotation *idAnnotation);

    /*!
     * @brief This helper function sets a interface symbol's unique id from the specified annotation.
     *
     * @param[inout] iface Function whose id will be modified.
     * @param[in] interfaceId Id value that will be assigned to fn.
     */
    void setInterfaceId(Interface *iface, Annotation *interfaceId);

    /*!
     * @brief Checks for duplicate ids within an interface
     *
     * This function will check for any conflicting id's, and will print a warning
     * message if any conflicts arise.
     *
     * @param[in] usedIds A list to keep track of what id values have been used.
     * @param[in] idType Which id is checked. Interface or function id.
     */
    void checkDuplicateIds(std::vector<idAndName_t> ids, std::string idType);

    /*!
     * @brief Prints id warning message
     *
     * @param[in] duplicateIds The list of ids that are not unique
     * @param[in] idType Which id is checked. Interface or function id.
     */
    void printDuplicateIdWarning(std::set<idAndName_t> duplicateIds, std::string idType);

    std::vector<idAndName_t> m_usedFunctionIds;  /*!< contains used function ids */
    std::vector<idAndName_t> m_usedInterfaceIds; /*!< contains used interface ids */
};
} // namespace erpcgen

#endif // _EMBEDDED_RPC__UNIQUEIDCHECKER_H_
