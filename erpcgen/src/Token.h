/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__TOKEN_H_
#define _EMBEDDED_RPC__TOKEN_H_

#include "Value.h"
#include "smart_ptr.h"

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen {

/*!
 * @brief Token location in the source file.
 */
struct token_loc_t
{
    int m_firstLine; /*!< Starting line of the token. */
    int m_firstChar; /*!< Starting character position on starting line. */
    int m_lastLine;  /*!< Ending line of the token. */
    int m_lastChar;  /*!< Ending character position on ending line. */

    token_loc_t()
    : m_firstLine(0)
    , m_firstChar(0)
    , m_lastLine(0)
    , m_lastChar(0)
    {
    }

    /*!
     * @brief Default copy constructor.
     *
     * @param[in] other Token location struct to copy.
     */
    token_loc_t(const token_loc_t &other) = default;

    /*!
     * @brief Default assign operator.
     *
     * @param[in] other Token location struct to copy.
     *
     * @return Token location struct reference.
     */
    token_loc_t &operator=(const token_loc_t &other) = default;
};

/*!
 * @brief Encapsulates all information about a token.
 */
class Token
{
public:
    /*!
     * @brief Constructor.
     *
     * This function set token number for this object to given token number.
     *
     * @param[in] token Given token number.
     *
     * @see Token::Token(int token, Value * value)
     * @see Token::Token(int token, Value * value, const token_loc_t & loc)
     * @see Token::Token(const Token & other)
     */
    Token(int token)
    : m_token(token)
    , m_value()
    , m_location()
    {
    }

    /*!
     * @brief Constructor.
     *
     * This function set token number to given token number and value to given
     * value for this object.
     *
     * @param[in] token Given token number.
     * @param[in] value Pointer to given value.
     *
     * @see Token::Token(int token)
     * @see Token::Token(int token, Value * value, const token_loc_t & loc)
     * @see Token::Token(const Token & other)
     */
    Token(int token, Value *value)
    : m_token(token)
    , m_value(value)
    , m_location()
    {
    }

    /*!
     * @brief Constructor.
     *
     * This function set token number to given token number, value to given value and location in file to
     * given location for this object.
     *
     * @param[in] token Given token number.
     * @param[in] value Pointer to given value.
     * @param[in] loc Struct with token location in file.
     *
     * @see Token::Token(int token)
     * @see Token::Token(int token, Value * value)
     * @see Token::Token(const Token & other)
     */
    Token(int token, Value *value, const token_loc_t &loc)
    : m_token(token)
    , m_value(value)
    , m_location(loc)
    {
    }

    /*!
     * @brief This function is copy constructor.
     *
     * This function copy value from given token to new token, when his
     * constructor is called (when token variable is declared).
     *
     * @param[in] other Token, which values are copied to this object.
     *
     * @see Token::Token(int token)
     * @see Token::Token(int token, Value * value)
     * @see Token::Token(int token, Value * value, const token_loc_t & loc)
     */
    Token(const Token &other);

    /*!
     * @brief This function copy data from one token to other token with using
     * assigned operator.
     *
     * The function copy all necessary data from token at right side to token
     * at left side, which are separated by operator "=".
     *
     * @param[in] other Token, which values are copied to this object.
     *
     * @return
     */
    Token &operator=(const Token &other);

    /*!
     * @brief Destructor.
     */
    ~Token() {}

    //! @name Token
    //@{
    /*!
     * @brief This function returns token number.
     *
     * @return Token number.
     *
     * @see void Token::setToken()
     */
    int getToken() const { return m_token; }

    /*!
     * @brief This function set token number.
     *
     * @param[in] tok Given token number.
     *
     * @see int Token::getToken()
     */
    void setToken(int tok) { m_token = tok; }

    /*!
     * @brief This function returns token name.
     *
     * The function returns token name for current token.
     *
     * @return Return token name.
     */
    const char *getTokenName() const;

    /*!
     * @brief Returns true if token is an integer or float literal.
     *
     * @retval True when token is number type, else false.
     */
    bool isNumberTok() const;

    /*!
     * @brief Returns true if token is an identifier
     *
     * @retval True when token is identifier type, else false.
     */
    bool isIdentifierTok() const;
    //@}

    //! @name Value
    //@{
    /*!
     * @brief This function return true/false, when token has/hasn't value.
     *
     * @retval true When token has value.
     * @retval false When token hasn't value.
     *
     * @see Value * Token::getValue()
     * @see const Value * Token::getValue()
     * @see void Token::setValue()
     */
    bool hasValue() const { return m_value.get() != nullptr; }

    /*!
     * @brief This function returns token value.
     *
     * @return Return token value.
     *
     * @see void Token::hasValue()
     * @see const Value * Token::getValue()
     * @see void Token::setValue()
     */
    Value *getValue() { return m_value; }

    /*!
     * @brief This function returns const token value.
     *
     * @return Return const token value.
     *
     * @see void Token::hasValue()
     * @see Value * Token::getValue()
     * @see void Token::setValue()
     */
    const Value *getValue() const { return m_value; }

    /*!
     * @brief This function set token value.
     *
     * The function set token value for current token.
     *
     * @param[in] val Pointer to given value.
     *
     * @see void Token::hasValue()
     * @see Value * Token::getValue()
     * @see const Value * Token::getValue()
     */
    void setValue(Value *val) { m_value = val; }

    /*!
     * @brief This function returns string value.
     *
     * The function returns string value converted from type value.
     *
     * @return Return string representation of value.
     *
     * @see uint32_t Token::getIntValue() const
     *
     * @exception internal_error Thrown if variable m_value is null.
     */
    const std::string &getStringValue() const;

    /*!
     * @brief This function returns integer value.
     *
     * The function returns integer value converted from type value.
     *
     * @return Return integer representation of value.
     *
     * @see const std::string & Token::getStringValue() const
     *
     * @exception internal_error Thrown if variable m_value is null.
     */
    uint64_t getIntValue() const;
    //@}

    //! @name Location
    //@{
    /*!
     * @brief This function returns token location (location from analyzed file).
     *
     * @return Return token location (location from analyzed file).
     *
     * @see void Token::setLocation()
     */
    const token_loc_t &getLocation() const { return m_location; }

    /*!
     * @brief This function set token location (location from analyzed file).
     *
     * @param[in] loc Token location (location from analyzed file).
     *
     * @see void Token::getLocation()
     */
    void setLocation(const token_loc_t &loc) { m_location = loc; }

    /*!
     * @brief This function set token location (location from analyzed file).
     *
     * @param[in] first Token locations, which values are copied to this object locations.
     * @param[in] last Token locations, which values are copied to this object locations.
     */
    void setLocation(const token_loc_t &first, const token_loc_t &last);

    /*!
     * @brief This function returns first line of token from token location
     * (location from analyzed file).
     *
     * @return Return first line of token location (location from analyzed file).
     *
     * @see void Token::getLastLine()
     */
    int getFirstLine() const { return m_location.m_firstLine; }

    /*!
     * @brief This function returns last line of token from token location
     * (location from analyzed file).
     *
     * @return Return last line of token location (location from analyzed file).
     *
     * @see void Token::getFirstLine()
     */
    int getLastLine() const { return m_location.m_lastLine; }

    /*!
     * @brief This function returns last line of token from token location
     * (location from analyzed file).
     *
     * @retval true Return true when token means binary operation.
     * @retval false Return false when token doesn't mean binary operation.
     */
    bool isBinaryOp() const;
    //@}

private:
    int m_token;              /*!< Token number */
    smart_ptr<Value> m_value; /*!< Token value */
    token_loc_t m_location;   /*!< Token location */
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__TOKEN_H_
