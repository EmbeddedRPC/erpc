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

#include "AliasType.h"
#include "Annotation.h"
#include "ArrayType.h"
#include "DataType.h"
#include "EnumMember.h"
#include "EnumType.h"
#include "Function.h"
#include "Interface.h"
#include "ListType.h"
#include "Logging.h"
#include "ParseErrors.h"
#include "StructMember.h"
#include "StructType.h"
#include "Symbol.h"
#include "SymbolScope.h"
#include "UnionCase.h"
#include "UnionType.h"
#include <string.h>

using namespace erpcgen;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

// Unique ID counters start at 1.
uint32_t Function::s_idCounter = 1;
uint32_t Interface::s_idCounter = 1;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

Value *Annotation::getValueObject()
{
    if (hasValue())
    {
        return m_value;
    }
    else
    {
        throw semantic_error(format_string("Missing value for annotation named '%s' on line '%d'", m_name.c_str(),
                                           m_location.m_firstLine));
    }
}

std::string Symbol::printAnnotations()
{
    std::string ret;
    ret += "Annotations [ ";
    int16_t annotationCount = (int16_t)m_annotations.size();
    int16_t annotationIndex = 1;
    for (auto annotation : m_annotations)
    {
        ret += annotation.toString();
        if (annotationIndex < annotationCount)
        {
            ret += ",";
        }
        ret += " ";
        ++annotationIndex;
    }
    ret += "]";
    return ret;
}

Annotation *Symbol::findAnnotation(std::string name)
{
    std::vector<Annotation *> annotationList = getAnnotations(name);
    if (0 < annotationList.size())
    {
        return annotationList.back();
    }
    else
    {
        return nullptr;
    }
}

std::vector<Annotation *> Symbol::getAnnotations(std::string name)
{
    std::vector<Annotation *> anList;
    for (int i = 0; i < m_annotations.size(); ++i)
    {
        if (m_annotations[i].getName() == name)
        {
            anList.push_back(&m_annotations[i]);
        }
    }
    return anList;
}

SymbolScope::typed_iterator::typed_iterator(const vit &bv, const vit &ev, Symbol::symbol_type_t predicateType)
: m_vec(bv)
, m_endvec(ev)
, m_predicateType(predicateType)
{
    // Advance to the first matching symbol.
    while (m_vec != m_endvec && (*m_vec)->getSymbolType() != m_predicateType)
    {
        ++m_vec;
    }
}

SymbolScope::typed_iterator &SymbolScope::typed_iterator::operator++()
{
    while (m_vec != m_endvec && (*m_vec)->getSymbolType() != m_predicateType)
    {
        ++m_vec;
    }
    return *this;
}

SymbolScope::typed_iterator SymbolScope::tbegin(Symbol::symbol_type_t predicateType)
{
    return typed_iterator(m_symbolVector.begin(), m_symbolVector.end(), predicateType);
}

SymbolScope::typed_iterator SymbolScope::tend(Symbol::symbol_type_t predicateType)
{
    return typed_iterator(m_symbolVector.end(), m_symbolVector.end(), predicateType);
}

SymbolScope::symbol_vector_t SymbolScope::getSymbolsOfType(Symbol::symbol_type_t predicateType)
{
    symbol_vector_t syms;
    for (auto it : m_symbolVector)
    {
        if (it->getSymbolType() == predicateType)
        {
            syms.push_back(it);
        }
    }
    return syms;
}

bool SymbolScope::hasSymbol(const std::string &name, bool recursive)
{
    bool isPresent = (m_symbolMap.find(name) != m_symbolMap.end());
    if (!isPresent && m_parent && recursive)
    {
        isPresent = m_parent->hasSymbol(name, true);
    }
    return isPresent;
}

Symbol *SymbolScope::getSymbol(const std::string &name, bool recursive)
{
    auto it = m_symbolMap.find(name);
    if (it != m_symbolMap.end())
    {
        return it->second;
    }
    else if (m_parent && recursive)
    {
        return m_parent->getSymbol(name, true);
    }
    else
    {
        return nullptr;
    }
}

void SymbolScope::addSymbol(Symbol *sym, int32_t pos)
{
    assert(sym);

    // Check for existing symbol with same name except structs. Structs are already added as typedef and it always call
    // semantic error.
    // sym->getName() == "" for anonymous struct and enums
    if (hasSymbol(sym->getName()) && sym->getName() != "")
    {
        Symbol *existing = getSymbol(sym->getName());
        if (existing->isBuiltin())
        {
            throw semantic_error(format_string("line %d: attempted redefinition of builtin symbol '%s'",
                                               sym->getFirstLine(), sym->getName().c_str()));
        }
        else
        {
            throw semantic_error(format_string("line %d: duplicate symbol with name '%s' (original on line %d)",
                                               sym->getFirstLine(), sym->getName().c_str(), existing->getFirstLine()));
        }
    }

    // Add the symbol.
    m_symbolMap[sym->getName()] = sym;
    if (pos >= 0)
    {
        m_symbolVector.insert(m_symbolVector.begin() + pos, sym);
    }
    else
    {
        m_symbolVector.push_back(sym);
    }
}

void SymbolScope::replaceSymbol(Symbol *oldSym, Symbol *newSym)
{
    int32_t symbolPos = getSymbolPos(oldSym);
    if (symbolPos >= 0)
    {
        m_symbolMap.erase(oldSym->getName());
        m_symbolVector.erase(m_symbolVector.begin() + symbolPos);
        addSymbol(newSym, symbolPos);
    }
    else
    {
        Log::warning("Symbol %s doesn't exist. Can't be replaced with %s.", oldSym->getName().c_str(),
                     newSym->getName().c_str());
    }
}

int32_t SymbolScope::getSymbolPos(Symbol *sym)
{
    for (int i = 0; i < m_symbolVector.size(); i++)
    {
        if (m_symbolVector[i] == sym)
        {
            return i;
        }
    }
    return -1;
}

void SymbolScope::dump()
{
    int n = 0;
    for (auto it : m_symbolVector)
    {
        Log::debug("%d: %s\n", n, it->getDescription().c_str());
        ++n;
    }
}

void SymbolScope::clear()
{
    m_symbolMap.clear();
    m_symbolVector.clear();
}

std::string ListType::getDescription() const
{
    return format_string("<list:%s>", m_elementType ? m_elementType->getDescription().c_str() : "(null)");
}

std::string ArrayType::getDescription() const
{
    return format_string("<array:%d:%s>", m_elementCount,
                         m_elementType ? m_elementType->getDescription().c_str() : "(null)");
}

bool StructType::containStringMember()
{
    for (StructMember *s : getMembers())
    {
        if (s->getContainString())
        {
            return true;
        }
    }
    return false;
}

bool StructType::containListMember()
{
    for (StructMember *s : getMembers())
    {
        if (s->getContainList())
        {
            return true;
        }
    }
    return false;
}

void StructType::addMember(StructMember *newMember)
{
    assert(newMember);

    m_scope.addSymbol(newMember);
    m_members.push_back(newMember);
}

std::string StructType::getDescription() const
{
    std::string members;
    int n = 0;
    for (auto it : m_members)
    {
        members += format_string("%d:", n);
        members += it->getDescription();
        if (n < m_members.size() - 1)
        {
            members += ", ";
        }
        ++n;
    }
    return format_string("<struct %s [%s]>", m_name.c_str(), members.c_str());
}

void StructType::addStructDirectionType(_param_direction structDirectionType)
{
    if (!hasStructDirectionType(structDirectionType))
    {
        m_structDirectionTypes.push_back(structDirectionType);
        for (auto it : m_members)
        {
            DataType *dataType = it->getDataType()->getTrueContainerDataType();
            if (dataType->isStruct())
            {
                StructType *a = dynamic_cast<StructType *>(dataType);
                assert(a);
                if (!a->hasStructDirectionType(structDirectionType))
                {
                    a->addStructDirectionType(structDirectionType);
                }
            }
            // TODO:
            /*if (dataType->isUnion())
            {
                UnionType *u = dynamic_cast<UnionType *>(dataType);
                for (auto it : u->getCases())
                {
                    // TODO: Must check for nullptr because the case could be the "void" case,
                    //   which does not point to a union member declaration
                    if (nullptr != it->getCaseMember())
                    {
                        DataType *caseDataType = it->getCaseMember()->getDataType()->getTrueContainerDataType();
                        if (caseDataType->isStruct())
                        {
                            StructType *b = dynamic_cast<StructType *>(caseDataType);
                            if (!b->hasStructDirectionType(structDirectionType))
                            {
                                b->addStructDirectionType(structDirectionType);
                            }
                        }
                    }
                }
            }*/
        }
    }
}

bool StructType::hasStructDirectionType(_param_direction structDirectionType)
{
    for (auto in : m_structDirectionTypes)
    {
        if (in == structDirectionType)
        {
            return true;
        }
    }
    return false;
}

bool StructType::structIsUsed()
{
    bool in = hasStructDirectionType(kInDirection);
    bool out = hasStructDirectionType(kOutDirection);
    bool outByref = hasStructDirectionType(kOutDirectionByref);
    bool inOut = hasStructDirectionType(kInoutDirection);
    bool ret = hasStructDirectionType(kReturn);

    return in || out || outByref || inOut || ret;
}

std::string StructMember::getDescription() const
{
    return format_string("<member %s:%s>", m_name.c_str(),
                         (m_dataType ? m_dataType->getDescription().c_str() : "(no type)"));
}

EnumMember *EnumType::getMember(std::string name)
{
    for (auto member : m_members)
    {
        if (0 == strcmp(member->getName().c_str(), name.c_str()))
        {
            return member;
        }
    }
    return nullptr;
}

void EnumType::addMember(EnumMember *newMember)
{
    assert(newMember);
    m_members.push_back(newMember);
}

std::string EnumType::getDescription() const
{
    std::string members;
    int n = 0;
    for (auto it : m_members)
    {
        members += format_string("%d:", n);
        members += it->getDescription();
        if (n < m_members.size() - 1)
        {
            members += ", ";
        }
        ++n;
    }
    return format_string("<enum %s [%s]>", m_name.c_str(), members.c_str());
}

std::string EnumMember::getDescription() const
{
    if (this->hasValue())
    {
        return format_string("<member %s:%d>", m_name.c_str(),
                             this->getValue()); //(m_dataType ? m_dataType->getDescription().c_str() : "(no type)"));
    }
    else
    {
        return format_string("<member %s>", m_name.c_str());
    }
}

std::string AliasType::getDescription() const
{
    return format_string("<type %s [%s]>", m_name.c_str(), m_elementType->getDescription().c_str());
}

DataType *DataType::getTrueDataType()
{
    if (this->isAlias())
    {
        AliasType *a = dynamic_cast<AliasType *>(this);
        assert(a);
        return a->getElementType()->getTrueDataType();
    }
    else
    {
        return this;
    }
}

DataType *DataType::getTrueContainerDataType()
{
    DataType *trueDataType = this->getTrueDataType();
    switch (trueDataType->getDataType())
    {
        case DataType::kListType:
        {
            ListType *l = dynamic_cast<ListType *>(trueDataType);
            assert(l);
            return l->getElementType()->getTrueContainerDataType();
        }
        case DataType::kArrayType:
        {
            ArrayType *a = dynamic_cast<ArrayType *>(trueDataType);
            assert(a);
            return a->getElementType()->getTrueContainerDataType();
        }
        default:
            return trueDataType;
    }
}

std::string Function::getDescription() const
{
    return format_string("<function(%u) %s->%s [%s]>", m_uniqueId, m_name.c_str(),
                         (m_returnType ? m_returnType->getDescription().c_str() : "(oneway)"),
                         m_parameters.getDescription().c_str());
}

void Interface::addFunction(Function *func)
{
    assert(func);

    m_scope.addSymbol(func);
    m_functions.push_back(func);
}

std::string Interface::getDescription() const
{
    std::string fns;
    int n = 0;
    for (auto it : m_functions)
    {
        fns += format_string("%d:", n);
        fns += it->getDescription();
        if (n < m_functions.size() - 1)
        {
            fns += ", ";
        }
        ++n;
    }
    return format_string("<interface(%u) %s [%s]>", m_uniqueId, m_name.c_str(), fns.c_str());
}

/* Union Functions */
/* UnionCase */

bool UnionCase::caseMemberIsVoid() const
{
    if (1 == m_memberDeclarationNames.size() && "void" == m_memberDeclarationNames[0])
    {
        return true;
    }
    return false;
}

std::string UnionCase::getDescription() const
{
    std::string description;
    std::string caseName = ("" != m_caseName) ? m_caseName : std::to_string(m_caseValue);
    description += "<" + caseName + ":";
    if (caseMemberIsVoid())
    {
        description += "(void)";
    }
    else
    {
        description += "{";
        for (auto memberName : m_memberDeclarationNames)
        {
            StructMember *member = m_containingUnion->getUnionMemberDeclaration(memberName);
            description += member->getName() + ":" + member->getDataType()->getName() + " ";
        }
        description += "}";
    }
    description += ">";
    return description;
}

StructMember *UnionCase::getUnionMemberDeclaration(const std::string &name)
{
    return m_containingUnion->getUnionMemberDeclaration(name);
}

/* UnionType */

void UnionType::addCase(UnionCase *unionCase)
{
    unionCase->setUnionReferencePointer(this);
    m_unionCases.push_back(unionCase);
}

std::string UnionType::getDescription() const
{
    std::string description = "Union:" + m_name;
    description += "[";
    for (auto caseMember : m_unionCases)
    {
        description += caseMember->getDescription();
    }
    description += "]";
    return description;
}

UnionType::case_vector_t UnionType::getUniqueCases()
{
    UnionType::case_vector_t uniqueCases;
    bool addCase = true;
    for (auto unionCase : getCases())
    {
        for (auto uniqueCase : uniqueCases)
        {
            if (casesAreTheSame(unionCase, uniqueCase))
            {
                addCase = false;
            }
        }
        if (addCase)
        {
            uniqueCases.push_back(unionCase);
        }
        addCase = true;
    }
    return uniqueCases;
}

bool UnionType::casesAreTheSame(UnionCase *a, UnionCase *b)
{
    std::vector<std::string> aNames = a->getMemberDeclarationNames();
    std::vector<std::string> bNames = b->getMemberDeclarationNames();
    if (aNames.size() != bNames.size())
    {
        return false;
    }
    for (int i = 0; i < aNames.size(); ++i)
    {
        if (aNames[i] != bNames[i])
        {
            return false;
        }
    }
    return true;
}

bool UnionType::addUnionMemberDeclaration(const std::string &name, DataType *dataType)
{
    if (declarationExists(name))
    {
        throw semantic_error(format_string("Redefinition of union member: '%s'\n", name.c_str()));
    }
    StructMember newMember = StructMember(name, dataType);
    m_caseMembers.push_back(newMember);
    return true;
}

bool UnionType::declarationExists(const std::string &name)
{
    for (auto member : m_caseMembers)
    {
        if (name == member.getName())
        {
            return true;
        }
    }
    return false;
}

StructMember *UnionType::getUnionMemberDeclaration(const std::string &name)
{
    for (auto &caseMember : m_caseMembers)
    {
        if (name == caseMember.getName())
        {
            return &caseMember;
        }
    }
    throw semantic_error(format_string("Union member not found: '%s'\n", name.c_str()));
}

void UnionType::printUnionMembers()
{
    for (auto member : m_caseMembers)
    {
        Log::debug("Member declaration:%s %s\n", member.getDataType()->getName().c_str(), member.getName().c_str());
    }
}
