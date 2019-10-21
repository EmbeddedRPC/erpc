/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__SYMBOL_H_
#define _EMBEDDED_RPC__SYMBOL_H_

#include "Annotation.h"
#include "AstNode.h"
#include "Token.h"
#include <vector>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen {

/*!
 * @brief Base class for all named declarations in the IDL.
 */
class Symbol
{
public:
    /*!
     *  @brief Supported symbol types.
     */
    enum symbol_type_t
    {
        kConstSymbol,
        kEnumMemberSymbol,
        kFunctionSymbol,
        kInterfaceSymbol,
        kProgramSymbol,
        kStructMemberSymbol,
        kTypenameSymbol,
        kUnionCaseMemberSymbol,

        kAliasTypeSymbol,
        kArrayTypeSymbol,
        kBuiltinTypeSymbol,
        kEnumTypeSymbol,
        kFunctionTypeSymbol,
        kListTypeSymbol,
        kStructTypeSymbol,
        kUnionTypeSymbol
    };

    /*!
     * @brief Constructor.
     *
     * This function set symbol type variable to given symbol type.
     *
     * @param[in] symType Enum symbol type.
     */
    Symbol(symbol_type_t symType)
    : m_symbolType(symType)
    , m_name()
    , m_location()
    , m_annotations()
    , m_mlComment("")
    , m_ilComment("")
    {
    }

    /*!
     * @brief Constructor.
     *
     * This function set symbol type variable to given symbol type and name to given name.
     *
     * @param[in] symType Enum symbol type.
     * @param[in] name Name for symbol.
     */
    Symbol(symbol_type_t symType, const std::string &name)
    : m_symbolType(symType)
    , m_name(name)
    , m_location()
    , m_annotations()
    , m_mlComment("")
    , m_ilComment("")
    {
    }

    /*!
     * @brief Constructor.
     *
     * This function set symbol type variable to given symbol type, name to token value and location to token location.
     *
     * @param[in] symType Enum symbol type.
     * @param[in] tok Token for symbol, which contains name and location.
     */
    Symbol(symbol_type_t symType, const Token &tok)
    : m_symbolType(symType)
    , m_name(tok.getStringValue())
    , m_location(tok.getLocation())
    , m_annotations()
    , m_mlComment("")
    , m_ilComment("")
    {
    }

    /*!
     * @brief Destructor.
     */
    virtual ~Symbol() {}

    /*!
     * @brief This function returns symbol type.
     *
     * @returns Return symbol type.
     */
    symbol_type_t getSymbolType() const { return m_symbolType; }

    /*!
     * @brief This function returns symbol name.
     *
     * @returns Return symbol name.
     */
    const std::string &getName() const { return m_name; }

    /*!
     * @brief This function set symbol name.
     *
     * @param[in] newName New name for symbol.
     */
    void setName(const std::string &newName) { m_name = newName; }

    /*!
     * @brief This function returns location for symbol.
     *
     * @returns Return location for symbol.
     */
    token_loc_t &getLocation() { return m_location; }

    /*!
     * @brief This function set location for symbol.
     *
     * @param[in] loc Symbol location.
     */
    void setLocation(const token_loc_t &loc) { m_location = loc; }

    /*!
     * @brief This function return first line from location of symbol.
     *
     * @returns Return first line from location of symbol.
     */
    int getFirstLine() const { return m_location.m_firstLine; }

    /*!
     * @brief This function return last line from location of symbol.
     *
     * @returns Return last line from location of symbol.
     */
    int getLastLine() const { return m_location.m_lastLine; }

    /*!
     * @brief This function return "false" value as default for identify builtin type.
     *
     * @returns Always return false.
     */
    virtual bool isBuiltin() const { return false; }

    /*!
     * @brief This function returns description about the symbol (symbol name).
     *
     * @return Symbol name.
     */
    virtual std::string getDescription() const { return m_name; }

    /*!
     * @brief This function add annotation to vector of symbol annotations.
     *
     * @param[in] a AstNode contains annotation information.
     */
    void addAnnotation(const Annotation &a) { m_annotations.push_back(a); }

    /*!
     * @brief This function returns description about annotation.
     *
     * @return Annotation description.
     */
    std::string printAnnotations();

    /*!
     * @brief Find annotation in the annotation list
     *
     * @param[in] name Annotation name.
     * @param[in] lang Programing language for which is annotation intended.
     *
     * @return An index into the annotation list
     */
    Annotation *findAnnotation(std::string name, Annotation::program_lang_t lang);

    /*!
     * @brief Find annotations matching name in the annotation list
     *
     * @param[in] name Annotation name.
     * @param[in] lang Programing language for which is annotation intended.
     *
     * @return A vector of matching annotations
     */
    std::vector<Annotation *> getAnnotations(std::string name, Annotation::program_lang_t lang);

    /*!
     * @brief Return all Symbol annotations.
     *
     * @return A vector of Symbol annotations.
     */
    const std::vector<Annotation> &getAnnotations() const { return m_annotations; };

    /*!
     * @brief This function search and returns Value object for given annotation name.
     *
     * @param[in] annName Given annotation name.
     * @param[in] lang Programing language for which is annotation intended.
     *
     * @return NULL if annotation is not found else value object.
     */
    Value *getAnnValue(const std::string annName, Annotation::program_lang_t lang);

    /*!
     * @brief This function search and returns string for given annotation name.
     *
     * @param[in] annName Given annotation name.
     * @param[in] lang Programing language for which is annotation intended.
     *
     * @return empty string if annotation is not found else string value.
     */
    std::string getAnnStringValue(const std::string annName, Annotation::program_lang_t lang);

    /*!
     * @brief This function returns multiline comment for this symbol declared in IDL file.
     *
     * @return Multiline comment for symbol.
     */
    std::string getMlComment() { return m_mlComment; }

    /*!
     * @brief This function set multiline comment for this symbol declared in IDL file.
     *
     * @param[in] comment Multiline comment for symbol.
     */
    void setMlComment(std::string comment) { m_mlComment = comment; }

    /*!
     * @brief This function returns inline comment for this symbol declared in IDL file.
     *
     * @return Inline comment for symbol.
     */
    std::string getIlComment() { return m_ilComment; }

    /*!
     * @brief This function set inline comment for this symbol declared in IDL file.
     *
     * @param[in] comment Inline comment for symbol.
     */
    void setIlComment(std::string comment) { m_ilComment = comment; }

protected:
    symbol_type_t m_symbolType;            /*!< Symbol type. */
    std::string m_name;                    /*!< Symbol name. */
    token_loc_t m_location;                /*!< Location in parsed file. */
    std::vector<Annotation> m_annotations; /*!< Vector of annotations belong to symbol. */
    std::string m_mlComment;               /*!< Comment above to symbol. */
    std::string m_ilComment;               /*!< Comment next to symbol. */
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__SYMBOL_H_
