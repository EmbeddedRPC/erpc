/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
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

#ifndef _EMBEDDED_RPC__UNIQUEIDCHECKER_H_
#define _EMBEDDED_RPC__UNIQUEIDCHECKER_H_
#include <stdio.h>
#include <vector>
#include <set>
#include <utility>
#include <string>

#include "InterfaceDefinition.h"
#include "Interface.h"
#include "SymbolScope.h"

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen
{
/*!
 * @brief Add ids to interfaces and functions.
 */
class UniqueIdChecker
{
public:
    typedef std::pair<int, std::string> idAndName_t;

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
}

#endif // _EMBEDDED_RPC__UNIQUEIDCHECKER_H_
