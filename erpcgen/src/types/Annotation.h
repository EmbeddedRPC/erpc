/*
 * The Clear BSD License
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 * that the following conditions are met:
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
