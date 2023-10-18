/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__GROUP_H_
#define _EMBEDDED_RPC__GROUP_H_

#include "Interface.hpp"
#include "cpptempl.hpp"

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
    typedef std::vector<Interface *> interface_list_t; /*!< Vector of interfaces belongs to group. */
    typedef std::vector<Symbol *> symbols_list_t;      /*!< Vector of symbols belongs to group. */
    typedef std::map<Symbol *, std::set<_param_direction>>
        symbol_directions_map_t; /*!< Map symbol with direction in which is used in current group. */

    /*!
     * @brief Constructor.
     *
     * @param[in] name Group name.
     */
    explicit Group(const std::string &name) : m_name(name) {}

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
    const std::string m_name;                   /*!< Group name. */
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__GROUP_H_
