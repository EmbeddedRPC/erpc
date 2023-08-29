/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__UNIONTYPE_H_
#define _EMBEDDED_RPC__UNIONTYPE_H_

#include "DataType.hpp"
#include "StructType.hpp"
#include "UnionCase.hpp"

#include <map>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen {

/*!
 * @brief Discriminated union data type.
 */
class UnionType : public DataType
{
public:
    typedef std::vector<UnionCase *> case_vector_t; /*!< Vector of Union cases. */

    /*!
     * @brief Constructor.
     *
     * This function set data type to union data type and name to given name.
     * It will also set discriminator name.
     *
     * @param[in] name Given name.
     * @param[in] discriminatorName Discriminator name.
     */
    UnionType(const std::string &name, const std::string &discriminatorName)
    : DataType(name, data_type_t::kUnionType)
    , m_discriminator(discriminatorName)
    , m_members("(union)")
    , m_parentStruct(nullptr)
    {
    }

    /*!
     * @brief Constructor.
     *
     * This function set data type to union data type and symbol token to given token.
     * It will also set discriminator name.
     *
     * @param[in] tok Given token..
     * @param[in] discriminatorName Discriminator name.
     */
    UnionType(const Token &tok, const std::string &discriminatorName)
    : DataType(tok, data_type_t::kUnionType)
    , m_discriminator(discriminatorName)
    , m_members("(union)")
    , m_parentStruct(nullptr)
    {
    }

    /*!
     * @brief Returns the struct owning this union.
     */
    StructType *getParentStruct() { return m_parentStruct; }

    /*!
     * @brief Sets the struct owning this union.
     */
    void setParentStruct(StructType *parent) { m_parentStruct = parent; }

    /*!
     * @brief This function will add union case to current union.
     *
     * @param[in] unionCase Given union case.
     */
    void addCase(UnionCase *unionCase);

    /*!
     * @brief @brief This function returns description about the union.
     *
     * @return String description about struct.
     *
     * @see std::string AliasType::getDescription() const
     * @see std::string EnumType::getDescription() const
     * @see std::string EnumMember::getDescription() const
     * @see std::string StructMember::getDescription() const
     * @see std::string VoidType::getDescription() const
     * @see std::string ArrayType::getDescription() const
     * @see std::string ListType::getDescription() const
     */
    virtual std::string getDescription() const override;

    /*!
     * @brief This function return "true" value for identify non-encapsulated discriminated union
     * type.
     *
     * @retval true True when m_discriminator is not set.
     */
    bool isNonEncapsulatedUnion() const { return m_discriminator.compare("") == 0; }

    /*!
     * @brief This function returns union cases vector.
     *
     * @return Vector of union cases vector.
     */
    case_vector_t &getCases() { return m_unionCases; }

    /*!
     * @brief This function returns unique union cases vector.
     *
     * @return Vector of union cases vector.
     */
    case_vector_t getUniqueCases();

    /*!
     * @brief This function adds unique union member declaration.
     *
     * @param[in] name Union case name.
     * @param[in] dataType Union case data type.
     */
    bool addUnionMemberDeclaration(const std::string &name, DataType *dataType);

    /*!
     * @brief This function returns struct of union members.
     *
     * @return Struct of union members.
     */
    StructType &getUnionMembers() { return m_members; }

    /*!
     * @brief This function returns union member declaration.
     *
     * @param[in] name Name of union member.
     *
     * @return Union member.
     */
    StructMember *getUnionMemberDeclaration(const std::string &name);

    /*!
     * @brief This function returns union discriminator name.
     *
     * @return Union discriminator name.
     */
    std::string getDiscriminatorName() { return m_discriminator; }

    /*!
     * @brief This function prints information about union members.
     */
    void printUnionMembers();

    /*!
     * @brief This function is looking for member declaration.
     *
     * @param[in] name Name of union member.
     *
     * @retval true when declaration already exist.
     * @retval false when declaration don't exist.
     */
    bool declarationExists(const std::string &name);

private:
    std::string m_discriminator; /*!< Name of union discriminator. */
    case_vector_t m_unionCases;  /*!< Contains union cases */
    StructType m_members;        /*!< Contains union members */
    StructType *m_parentStruct;  /*!< Struct containing this union */

    /*!
     * @brief This function is comparing two union cases.
     *
     * @param[in] a One union case.
     * @param[in] b Second union case.
     *
     * @retval true when union cases are same.
     * @retval false when union cases are not same.
     */
    bool casesAreTheSame(UnionCase *a, UnionCase *b);
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__UNIONTYPE_H_
