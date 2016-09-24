/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
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
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
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

#ifndef _EMBEDDED_RPC__UNIONCASE_H_
#define _EMBEDDED_RPC__UNIONCASE_H_
#include <map>
#include <utility>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen
{
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
    : Symbol(kUnionCaseTypeSymbol, caseName)
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
    : Symbol(kUnionCaseTypeSymbol)
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
    : Symbol(kUnionCaseTypeSymbol)
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
    uint32_t getCaseValue() const { return m_caseValue; }
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
