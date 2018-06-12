/*
 * The Clear BSD License
 * Copyright 2017 NXP
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

#ifndef _EMBEDDED_RPC__GROUP_H_
#define _EMBEDDED_RPC__GROUP_H_

#include "Interface.h"
#include "cpptempl.h"
#include <set>
#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen {

/*!
 * @brief An interface that contains functions.
 */
class Group
{
public:
    typedef std::vector<Interface *> interface_list_t;
    typedef std::vector<Symbol *> symbols_list_t;
    typedef std::map<Symbol *, std::set<_param_direction> > symbol_directions_map_t;

    /*!
     * @brief Constructor.
     *
     * This function set symbol token to given token.
     *
     * @param[in] tok Given token.
     */
    Group(const std::string name)
    : m_name(name)
    {
    }

    /*!
     * @brief This function returns the group name.
     *
     * @return String with the group name.
     */
    const std::string &getName() const { return m_name; }

    /*!
     * @brief This function will add pointer to interface into vector of group interfaces.
     *
     * @param[in] iface Pointer to interface.
     */
    void addInterface(Interface *iface);

    /*!
     * @brief This function returns vector with pointers to interfaces belonging to the group.
     *
     * @return Vector with pointers to interfaces belonging to the group.
     */
    const interface_list_t &getInterfaces() const { return m_interfaces; }

    /*!
     * @brief This function will add direction for specified symbol held in a symbol map.
     *
     * @param[in] symbol Symbol for which direction should be added.
     * @param[in] dir Direction of the symbol.
     */
    void addDirToSymbolsMap(Symbol *symbol, _param_direction dir);

    /*!
     * @brief This function returns set with symbol directions.
     *
     * @param[in] symbol Symbol which direction set should be returned.
     *
     * @return Set with symbol directions.
     */
    const std::set<_param_direction> getSymbolDirections(Symbol *symbol) const;

    /*!
     * @brief This function returns vector with symbols.
     *
     * Unique symbols are added into vector within addDirToSymbolsMap() function.
     *
     * @return Vector with pointers to symbols.
     */
    const symbols_list_t &getSymbols() const { return m_symbols; }

    /*!
     * @brief This function will set group template.
     *
     * @param[in] groupTemplate Group template defined as data map.
     */
    void setTemplate(cpptempl::data_map groupTemplate);

    /*!
     * @brief This function returns group template.
     *
     * @return Data map with group template.
     */
    cpptempl::data_map &getTemplate() { return m_template; }

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
    std::string getDescription() const;

private:
    interface_list_t m_interfaces;              /*!< Vector of interface functions. */
    symbol_directions_map_t m_symbolDirections; /*!< Map of symbol's directions. */
    symbols_list_t m_symbols;                   /*!< Vector of symbols in order of their IDL definition. */
    cpptempl::data_map m_template;              /*!< Data map with group template. */
    std::string m_name;                         /*!< Group name. */
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__GROUP_H_
