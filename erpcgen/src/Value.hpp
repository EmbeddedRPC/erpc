/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#if !defined(_Value_h_)
#define _Value_h_

#include "format_string.hpp"

#include <cstdint>
#include <string>

typedef enum
{
    kIntegerValue,
    kStringValue,
    kFloatValue
} value_type_t; /*!< Value types */

/*!
 * @brief Abstract base class for values of arbitrary types.
 */
class Value
{
public:
    /*!
     * @brief Constructor.
     *
     * @param[in] theType Value type.
     */
    Value(value_type_t theType)
    : m_type(theType)
    {
    }

    /*!
     * @brief Destructor.
     */
    virtual ~Value() {}

    /*!
     * @brief Get Value type.
     *
     * @return Value type.
     */
    virtual value_type_t getType() const { return m_type; }

    /*!
     * @brief Get Value type name.
     *
     * @return Value type name.
     */
    virtual std::string getTypeName() const = 0;

    /*!
     * @brief Get Value type size.
     *
     * @return Value type size.
     */
    virtual size_t getSize() const = 0;

    /*!
     * @brief Get Value type string representation.
     *
     * @return Value type string representation.
     */
    virtual std::string toString() const = 0;

    /*!
     * @brief Clone Value.
     *
     * @return Cloned Value.
     */
    virtual Value *clone() const = 0;

private:
    value_type_t m_type; /*!< Value type. */
};

/*!
 * @brief 64-bit integer value.
 */
class IntegerValue : public Value
{
public:
    //! Supported sizes of integers.
    typedef enum
    {
        kSigned,
        kSignedLong,
        kUnsigned,
        kUnsignedLong
    } int_type_t; //!< The integer type.

    /*!
     * @brief Constructor.
     */
    IntegerValue(int_type_t type = kSigned)
    : Value(kIntegerValue)
    , m_value(0)
    , m_intType(type)
    {
    }

    /*!
     * @brief Constructor.
     *
     * @param[in] value IntegerValue value.
     */
    IntegerValue(uint64_t value, int_type_t type = kSigned)
    : Value(kIntegerValue)
    , m_value(value)
    , m_intType(type)
    {
    }

    /*!
     * @brief Copy constructor.
     *
     * @param[in] other IntegerValue to copy.
     */
    IntegerValue(const IntegerValue &other)
    : Value(other.getType())
    , m_value(other.m_value)
    , m_intType(other.m_intType)
    {
    }

    /*!
     * @brief Get IntegerValue type name.
     *
     * @return IntegerValue type name.
     */
    virtual std::string getTypeName() const { return "integer"; }

    /*!
     * @brief Get IntegerValue type size.
     *
     * @return IntegerValue type size.
     */
    virtual size_t getSize() const { return sizeof(m_value); }

    /*!
     * @brief This function returns value.
     *
     * @return value IntegerValue value.
     */
    uint64_t getValue() const
    {
        return (m_intType == kSignedLong || m_intType == kUnsignedLong) ? m_value : (uint32_t)m_value;
    }

    /*!
     * @brief This function returns signed/unsigned type.
     *
     * @return value IntegerValue type.
     */
    int_type_t getIntType() { return m_intType; }

    /*!
     * @brief Casting operator.
     *
     * @return value IntegerValue value.
     */
    operator uint64_t() const { return m_value; }

    /*!
     * @brief Assign operator.
     *
     * @param[in] value IntegerValue value.
     *
     * @return IntegerValue reference.
     */
    IntegerValue &operator=(int64_t value)
    {
        m_value = value;
        return *this;
    }

    /*!
     * @brief Get IntegerValue type string representation.
     *
     * @return IntegerValue type string representation.
     */
    virtual std::string toString() const
    {
        if (m_intType == kUnsigned)
            return format_string("%uU", (uint32_t)m_value);
        else if (m_intType == kSigned)
            return format_string("%d", (int32_t)m_value);
        else if (m_intType == kUnsignedLong)
            return format_string("%lluU", m_value);
        else
            return format_string("%lld", (int64_t)m_value);
    }

    /*!
     * @brief Clone IntegerValue.
     *
     * @return Cloned IntegerValue.
     */
    virtual Value *clone() const { return new IntegerValue(*this); }

protected:
    uint64_t m_value;     //!< The integer value.
    int_type_t m_intType; //!< The integer type.
};

/*!
 * @brief Double floating point value.
 */
class FloatValue : public Value
{
public:
    /*!
     * @brief Constructor.
     */
    FloatValue()
    : Value(kFloatValue)
    , m_value(0.0)
    {
    }

    /*!
     * @brief Constructor.
     *
     * @param[in] value FloatValue value.
     */
    FloatValue(double value)
    : Value(kFloatValue)
    , m_value(value)
    {
    }

    /*!
     * @brief Constructor.
     *
     * @param[in] value FloatValue value.
     */
    FloatValue(float value)
    : Value(kFloatValue)
    , m_value(value)
    {
    }

    /*!
     * @brief Copy constructor.
     *
     * @param[in] other FloatValue to copy.
     */
    FloatValue(const FloatValue &other)
    : Value(kFloatValue)
    , m_value(other.m_value)
    {
    }

    /*!
     * @brief Assign operator.
     *
     * @param[in] other FloatValue to copy.
     *
     * @return FloatValue reference.
     */
    FloatValue &operator=(const FloatValue &other)
    {
        m_value = other.m_value;
        return *this;
    }

    /*!
     * @brief Get FloatValue type name.
     *
     * @return FloatValue type name.
     */
    virtual std::string getTypeName() const { return "float"; }

    /*!
     * @brief Get FloatValue type size.
     *
     * @return FloatValue type size.
     */
    virtual size_t getSize() const { return sizeof(m_value); }

    /*!
     * @brief This function returns value.
     *
     * @return value FloatValue value.
     */
    double getValue() const { return m_value; }

    /*!
     * @brief Casting operator returns value.
     *
     * @return value FloatValue value.
     */
    operator double() const { return m_value; }

    /*!
     * @brief Casting operator returns value.
     *
     * @return value FloatValue value.
     */
    operator float() const { return static_cast<float>(m_value); }

    /*!
     * @brief Assign operator.
     *
     * @param[in] value FloatValue value.
     *
     * @return FloatValue reference.
     */
    FloatValue &operator=(double value)
    {
        m_value = value;
        return *this;
    }

    /*!
     * @brief Assign operator.
     *
     * @param[in] value FloatValue value.
     *
     * @return FloatValue reference.
     */
    FloatValue &operator=(float value)
    {
        m_value = value;
        return *this;
    }

    /*!
     * @brief Get FloatValue type string representation.
     *
     * @return FloatValue type string representation.
     */
    virtual std::string toString() const { return format_string("%g", m_value); }

    /*!
     * @brief Clone FloatValue.
     *
     * @return Cloned FloatValue.
     */
    virtual Value *clone() const { return new FloatValue(*this); }

protected:
    double m_value; //!< The double value.
};

/*!
 * @brief String value.
 *
 * Simply wraps the STL std::string class.
 */
class StringValue : public Value
{
public:
    /*!
     * @brief Constructor.
     */
    StringValue()
    : Value(kStringValue)
    , m_value()
    {
    }

    /*!
     * @brief Constructor.
     *
     * @param[in] value StringValue value.
     */
    StringValue(const std::string &value)
    : Value(kStringValue)
    , m_value(value)
    {
    }

    /*!
     * @brief Constructor.
     *
     * @param[in] value StringValue value.
     */
    StringValue(const std::string *value)
    : Value(kStringValue)
    , m_value(*value)
    {
    }

    /*!
     * @brief Copy constructor.
     *
     * @param[in] other StringValue to copy.
     */
    StringValue(const StringValue &other)
    : Value(kStringValue)
    , m_value(other.m_value)
    {
    }

    /*!
     * @brief Get StringValue type name.
     *
     * @return StringValue type name.
     */
    virtual std::string getTypeName() const { return "string"; }

    /*!
     * @brief Get StringValue type size.
     *
     * @return StringValue type size.
     */
    virtual size_t getSize() const { return m_value.size(); }

    /*!
     * @brief Get StringValue value.
     *
     * @return StringValue value.
     */
    const std::string &getString() const { return m_value; }

    /*!
     * @brief Get StringValue value.
     *
     * @return StringValue value.
     */
    operator const char *() const { return m_value.c_str(); }

    /*!
     * @brief Get StringValue value.
     *
     * @return StringValue value.
     */
    operator const std::string &() const { return m_value; }

    /*!
     * @brief Get StringValue value.
     *
     * @return StringValue value.
     */
    operator std::string &() { return m_value; }

    /*!
     * @brief Get StringValue value.
     *
     * @return StringValue value.
     */
    operator const std::string *() { return &m_value; }

    /*!
     * @brief Get StringValue value.
     *
     * @return StringValue value.
     */
    operator std::string *() { return &m_value; }

    /*!
     * @brief Assign operator.
     *
     * @param[in] other StringValue to copy.
     *
     * @return StringValue reference.
     */
    StringValue &operator=(const StringValue &other)
    {
        m_value = other.m_value;
        return *this;
    }

    /*!
     * @brief Assign operator.
     *
     * @param[in] value StringValue value.
     *
     * @return StringValue reference.
     */
    StringValue &operator=(const std::string &value)
    {
        m_value = value;
        return *this;
    }

    /*!
     * @brief Assign operator.
     *
     * @param[in] value StringValue value.
     *
     * @return StringValue reference.
     */
    StringValue &operator=(const char *value)
    {
        m_value = value;
        return *this;
    }

    /*!
     * @brief Get StringValue type string representation.
     *
     * @return StringValue type string representation.
     */
    virtual std::string toString() const { return m_value; }

    /*!
     * @brief Clone StringValue.
     *
     * @return Cloned StringValue.
     */
    virtual Value *clone() const { return new StringValue(*this); }

protected:
    std::string m_value; //!< The string value.
};

#endif // _Value_h_
