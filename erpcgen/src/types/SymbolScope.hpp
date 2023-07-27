/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__SYMBOLSCOPE_H_
#define _EMBEDDED_RPC__SYMBOLSCOPE_H_

#include "Symbol.hpp"

#include <map>
#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen {

/*!
 * @brief A scoped namespace containing symbols.
 */
class SymbolScope
{
public:
    typedef std::map<std::string, Symbol *> symbol_map_t; /*!< Map symbol name to symbol type. */
    typedef std::vector<Symbol *> symbol_vector_t;        /*!< Vector of symbol types. */

    class typed_iterator
    {
    public:
        typedef symbol_vector_t::iterator vit;
        typedef vit::value_type value_type;

        operator value_type &() { return *m_vec; }
        value_type &operator*() { return *m_vec; }
        value_type &operator->() { return *m_vec; }

        /*!
         * @brief
         */
        typed_iterator &operator++();

        bool operator==(const typed_iterator &other) { return (m_vec == other.m_vec); }
        bool operator!=(const typed_iterator &other) { return !(*this == other); }

    private:
        symbol_vector_t::iterator m_vec;
        symbol_vector_t::iterator m_endvec;
        Symbol::symbol_type_t m_predicateType;

        /*!
         * @brief
         *
         * @param[in] bv
         * @param[in] ev
         * @param[in] predicateType
         */
        typed_iterator(const vit &bv, const vit &ev, Symbol::symbol_type_t predicateType);

        friend class SymbolScope;
    };

    /*!
     * @brief Constructor.
     */
    SymbolScope()
    : m_symbolMap()
    , m_symbolVector()
    , m_parent(nullptr)
    {
    }

    /*!
     * @brief Destructor.
     */
    virtual ~SymbolScope() {}

    /*!
     * @brief This function returns true when name is found in symbol map.
     *
     * This function returns true when name is found in symbol map. Symbol is found using given name.
     *
     * @param[in] name Name, which is used to find symbol.
     * @param[in] recursive If true, then recursive finding is allowed.
     *
     * @retval true (symbol is found)
     * @retval false (symbol isn't found)
     *
     * @see void SymbolScope::addSymbol()
     * @see Symbol * SymbolScope::getSymbol()
     * @see void SymbolScope::replaceSymbol()
     */
    bool hasSymbol(const std::string &name, bool recursive = true);

    /*!
     * @brief This function will return symbol.
     *
     * This function returns symbol found in symbol scope map m_symbolMap. Symbol is found using given name.
     *
     * @param[in] name Name, which is used to find symbol.
     * @param[in] recursive If true, then recursive finding is allowed.
     *
     * @return Requested symbol from symbol vector.
     * @retval nullptr (when symbol wasn't found)
     *
     * @see void SymbolScope::addSymbol()
     * @see bool SymbolScope::hasSymbol()
     * @see void SymbolScope::replaceSymbol()
     */
    Symbol *getSymbol(const std::string &name, bool recursive = true);

    /*!
     * @brief This function will add symbol to the symbol scope.
     *
     * The function will add symbol given by pointer sym to the symbol scope vector m_symbolVector.
     * If symbol with same name is already added, will be throw semantic error.
     *
     * @param[in] sym Symbol pointer, which is added to symbol scope vector.
     * @param[in] pos Position where symbol should be added. Default -1 mean at the end of vector.
     *
     * @see Symbol * SymbolScope::getSymbol()
     * @see bool SymbolScope::hasSymbol()
     * @see void SymbolScope::replaceSymbol()
     */
    void addSymbol(Symbol *sym, int32_t pos = -1);

    /*!
     * @brief This function will replace old symbol with new symbol in the symbol scope.
     *
     * The function will remove old symbol and call addSymbol with new symbol.
     *
     * @param[in] oldSym Symbol pointer, which will be replaced.
     * @param[in] newSym Symbol pointer, which will be replacer.
     *
     * @see Symbol * SymbolScope::addSymbol()
     * @see Symbol * SymbolScope::getSymbol()
     * @see bool SymbolScope::hasSymbol()
     */
    void replaceSymbol(Symbol *oldSym, Symbol *newSym);

    /*!
     * @brief This function will return symbol position.
     *
     * The function will return symbol position of given symbol by pointer sym from the
     * symbol scope vector m_symbolVector.
     *
     * @param[in] sym Pointer of symbol, which position is searched.
     *
     * @return -1 if position is not found, else position number.
     *
     * @see Symbol * SymbolScope::addSymbol()
     */
    int32_t getSymbolPos(const Symbol *sym);

    /*!
     * @brief Return boolean representation about existence of parent.
     *
     * @retval true When parent is set.
     * @retval false When parent isn't set.
     */
    bool hasParent() const { return m_parent != nullptr; }

    /*!
     * @brief Return pointer to parent.
     *
     * @return Pointer to parent.
     */
    SymbolScope *getParent() { return m_parent; }

    /*!
     * @brief This function set parent for this object.
     *
     * @param[in] parent Pointer to given parent.
     */
    void setParent(SymbolScope *parent) { m_parent = parent; }

    /*!
     * @brief Return begin of symbol vector.
     *
     * @return Return begin of symbol vector.
     */
    symbol_vector_t::iterator begin() { return m_symbolVector.begin(); }

    /*!
     * @brief Return end of symbol vector.
     *
     * @return Return end of symbol vector.
     */
    symbol_vector_t::iterator end() { return m_symbolVector.end(); }

    /*!
     * @brief
     *
     * @param[in] predicateType
     */
    typed_iterator tbegin(Symbol::symbol_type_t predicateType);

    /*!
     * @brief
     *
     * @param[in] predicateType
     */
    typed_iterator tend(Symbol::symbol_type_t predicateType);

    /*!
     * @brief This function returns symbol vector with symbols.
     *
     * This function returns symbol vector with symbols, which are requested. Symbols are found using given
     * symbol_type_t.
     *
     * @param[in] predicateType Symbol type, which is requested.
     *
     * @return Symbol vector with requested symbols.
     */
    symbol_vector_t getSymbolsOfType(Symbol::symbol_type_t predicateType);

    /*!
     * @brief This function returns symbol vector.
     *
     * @return Symbol vector with all symbols.
     */
    symbol_vector_t getSymbolVector() { return m_symbolVector; }

    /*!
     * @brief This function show debug description about the all symbols.
     *
     * @see std::string Interface::getDescription() const
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
    void dump();

    /*!
     * @brief Erase data from m_symbolMap and m_symbolVector
     */
    void clear();

protected:
    symbol_map_t m_symbolMap;       /*!< Map Symbols to names. */
    symbol_vector_t m_symbolVector; /*!< Vector of Symbols */
    SymbolScope *m_parent;          /*!< Scope of Symbol */
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__SYMBOLSCOPE_H_
