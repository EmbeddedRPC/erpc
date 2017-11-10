/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
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
 * o Neither the name of the copyright holder nor the names of its
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
     * @brief This function returns symbol output name.
     * Can be different to getName() when @name is used.
     *
     * @returns Return symbol output name.
     */
    std::string getOutputName();

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
     * @return An index into the annotation list
     */
    Annotation *findAnnotation(std::string name);

    /*!
     * @brief Find annotations matching name in the annotation list
     *
     * @return A vector of matching annotations
     */
    std::vector<Annotation *> getAnnotations(std::string name);

    /*!
     * @brief Find annotations matching name in the annotation list
     *
     * @return A vector of matching annotations
     */
    const std::vector<Annotation> &getAnnotations() const { return m_annotations; };

    /*!
     * @brief This function search and returns Value object for given annotation name.
     *
     * @param[in] annName Given annotation name.
     *
     * @return NULL if annotation is not found else value object.
     */
    Value *getAnnValue(const std::string annName);

    /*!
     * @brief This function search and returns string for given annotation name.
     *
     * @param[in] ann Given annotation name.
     *
     * @return empty string if annotation is not found else string value.
     */
    std::string getAnnStringValue(const std::string annName);

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
