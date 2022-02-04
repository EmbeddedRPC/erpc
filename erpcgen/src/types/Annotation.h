/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__ANNOTATION_H_
#define _EMBEDDED_RPC__ANNOTATION_H_

#include "AstNode.h"
#include "Token.h"
#include "Value.h"

#include <string>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen {

/*!
 * @brief Annotation class
 */
class Annotation
{
public:
    enum program_lang_t
    {
        kAll,
        kC,
        kPython
    };

    /*!
     * @brief Constructor.
     *
     * This function initializes object attributes from given Token and Value.
     *
     * @param[in] token Token contains annotation name and location in parsed file.
     * @param[in] val Pointer to value.
     * @param[in] lang Programming language for which is annotation intended.
     */
    Annotation(const Token &token, Value *val, program_lang_t lang)
    : m_name(token.getStringValue())
    , m_value(val)
    , m_location(token.getLocation())
    , m_lang(lang)
    {
    }

    /*!
     * @brief Constructor.
     *
     * This function initializes object attributes from given Token.
     *
     * @param[in] token  Token contains annotation name and location in parsed file.
     */
    Annotation(const Token &token)
    : m_name(token.getStringValue())
    , m_value(nullptr)
    , m_location(token.getLocation())
    , m_lang(kAll)
    {
    }

    /*!
     * @brief Constructor.
     *
     * This function initializes object attributes from given annotation.
     *
     * @param[in] a Source annotation.
     */
    Annotation(const Annotation &a)
    : m_name(a.m_name)
    , m_value(a.m_value)
    , m_location(a.m_location)
    , m_lang(a.m_lang)
    {
    }

    /*!
     * @brief This function returns annotation name.
     *
     * @return Annotation name.
     */
    std::string getName() const { return m_name; }

    /*!
     * @brief Checks to see if value instance member is null
     *
     * @retval True if m_value is not null
     */
    bool hasValue() { return nullptr != m_value; }

    /*!
     * @brief This function returns annotation value.
     *
     * @return Pointer to annotation value.
     *
     * @throw semantic_error Function will throw semantic error if hasValue() == false.
     */
    Value *getValueObject();

    /*!
     * @brief This function returns programming language type for which is annotation intended.
     *
     * @return Programming language type.
     */
    program_lang_t getLang() const { return m_lang; }

    /*!
     * @brief This function returns toString representation.
     *
     * @return Returns "name = value".
     */
    std::string toString() { return m_name + " = " + m_value->toString(); }

    /*!
     * @brief This function returns location for symbol.
     *
     * @returns Return location for symbol.
     */
    token_loc_t &getLocation() { return m_location; }

private:
    std::string m_name;     /*!< Annotation name. */
    Value *m_value;         /*!< Value of annotation*/
    token_loc_t m_location; /*!< Location of annotation in parsed file. */
    program_lang_t m_lang;  /*!< Programming language for which is annotation intended. */
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__ANNOTATION_H_
