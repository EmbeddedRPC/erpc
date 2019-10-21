/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__UNIONCASE_H_
#define _EMBEDDED_RPC__UNIONCASE_H_
#include <map>
#include <utility>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen {

class UnionType;

/*!
 * @brief Discriminated union case type.
 */
class UnionCase : public Symbol
{
public:
    /*!
     * @brief Constructor.
     *
     * This function set symbol to union case and name to given name.
     * It will also set case value.
     *
     * @param[in] caseName Given case name.
     * @param[in] caseValue Given case value.
     */
    UnionCase(const std::string &caseName, int32_t caseValue)
    : Symbol(kUnionCaseMemberSymbol, caseName)
    , m_caseName(caseName)
    , m_caseValue(caseValue)
    , m_containingUnion(nullptr)
    {
    }

    /*!
     * @brief Constructor.
     *
     * This function set symbol to union case.
     * It will also set case value.
     *
     * @param[in] caseValue Given case value.
     */
    UnionCase(int32_t caseValue)
    : Symbol(kUnionCaseMemberSymbol)
    , m_caseName("")
    , m_caseValue(caseValue)
    , m_containingUnion(nullptr)
    {
    }

    /*!
     * @brief Constructor.
     *
     * This function set symbol to union case and name to given name.
     * It will also set case value to -1 (unset).
     *
     * @param[in] caseName Given case name.
     */
    UnionCase(const std::string &caseName)
    : Symbol(kUnionCaseMemberSymbol)
    , m_caseName(caseName)
    , m_caseValue(-1)
    , m_containingUnion(nullptr)
    {
    }

    /*!
     * @brief This function set reference to union type.
     *
     * @param[in] unionType Pointer to union type.
     */
    void setUnionReferencePointer(UnionType *unionType) { m_containingUnion = unionType; }

    /*!
     * @brief This function returns union case value.
     *
     * @return Union case value.
     */
    int32_t getCaseValue() const { return m_caseValue; }

    /*!
     * @brief This function returns union case name.
     *
     * @return Union case name.
     */
    std::string getCaseName() { return m_caseName; }

    /*!
     * @brief This function returns vector of member declarations.
     *
     * @return Vector of member declarations.
     */
    std::vector<std::string> getMemberDeclarationNames() const { return m_memberDeclarationNames; }

    /*!
     * @brief This function adds case member name.
     *
     * param[in] name Case member name.
     */
    void addCaseMemberName(const std::string &name) { m_memberDeclarationNames.push_back(name); };

    /*!
     * @brief This function prints debug information about union case members.
     */
    void printUnionMembers();

    /*!
     * @brief This function returns information if case is void type.
     *
     * @retval true When case is void type.
     * @retval false When case contains not void type.
     */
    bool caseMemberIsVoid() const;

    /*!
     * @brief This function returns description about case members.
     *
     * @return Case description.
     */
    virtual std::string getDescription() const;

    /*!
     * @brief This function returns union member declaration.
     *
     * param[in] name Union member name.
     *
     * @return Union member.
     */
    StructMember *getUnionMemberDeclaration(const std::string &name);

private:
    std::string m_caseName;                            /*!< Union case name. */
    int32_t m_caseValue;                               /*!< Union case value. */
    std::vector<std::string> m_memberDeclarationNames; /*!< Vector of case members. */
    UnionType *m_containingUnion;                      /*!< Pointer to union type if it contains. */
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__UNIONCASE_H_
