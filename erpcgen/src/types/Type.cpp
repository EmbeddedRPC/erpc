/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "AliasType.hpp"
#include "Annotation.hpp"
#include "ArrayType.hpp"
#include "DataType.hpp"
#include "EnumMember.hpp"
#include "EnumType.hpp"
#include "Function.hpp"
#include "FunctionType.hpp"
#include "Group.hpp"
#include "Interface.hpp"
#include "ListType.hpp"
#include "Logging.hpp"
#include "ParseErrors.hpp"
#include "StructMember.hpp"
#include "StructType.hpp"
#include "Symbol.hpp"
#include "SymbolScope.hpp"
#include "UnionCase.hpp"
#include "UnionType.hpp"
#include "annotations.h"
#include "cpptempl.hpp"

#include <algorithm>
#include <cstring>

using namespace erpcgen;
using namespace std;

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

string Symbol::printAnnotations()
{
    string ret;
    ret += "Annotations [ ";
    int16_t annotationCount = (int16_t)m_annotations.size();
    int16_t annotationIndex = 1;
    for (auto &annotation : m_annotations)
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

Annotation *Symbol::findAnnotation(const string &name, Annotation::program_lang_t lang)
{
    vector<Annotation *> annotationList = getAnnotations(name, lang);
    if (0 < annotationList.size())
    {
        return annotationList.back();
    }
    else
    {
        return nullptr;
    }
}

vector<Annotation *> Symbol::getAnnotations(const string &name, Annotation::program_lang_t lang)
{
    vector<Annotation *> anList;
    for (unsigned int i = 0; i < m_annotations.size(); ++i)
    {
        if (m_annotations[i].getName() == name &&
            (m_annotations[i].getLang() == lang || m_annotations[i].getLang() == Annotation::program_lang_t::kAll))
        {
            anList.push_back(&m_annotations[i]);
        }
    }
    return anList;
}

Value *Symbol::getAnnValue(const string &annName, Annotation::program_lang_t lang)
{
    Annotation *ann = findAnnotation(annName, lang);
    return (ann) ? ann->getValueObject() : nullptr;
}

string Symbol::getAnnStringValue(const string &annName, Annotation::program_lang_t lang)
{
    Value *annVallue = getAnnValue(annName, lang);
    return (annVallue) ? annVallue->toString() : "";
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

bool SymbolScope::hasSymbol(const string &name, bool recursive)
{
    bool isPresent = (m_symbolMap.find(name) != m_symbolMap.end());
    if (!isPresent && m_parent && recursive)
    {
        isPresent = m_parent->hasSymbol(name, true);
    }
    return isPresent;
}

Symbol *SymbolScope::getSymbol(const string &name, bool recursive)
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

    // Check for existing symbol with same name.
    // sym->getName() == "" for anonymous struct and enums
    if (hasSymbol(sym->getName()) && sym->getName() != "")
    {
        Symbol *existing = getSymbol(sym->getName());
        if (existing->isDatatypeSymbol() && dynamic_cast<DataType *>(existing)->isBuiltin())
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

int32_t SymbolScope::getSymbolPos(const Symbol *sym)
{
    for (unsigned int i = 0; i < m_symbolVector.size(); i++)
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

string ListType::getDescription() const
{
    return format_string("<list:%s>", m_elementType ? m_elementType->getDescription().c_str() : "(null)");
}

string ArrayType::getDescription() const
{
    return format_string("<array:%d:%s>", m_elementCount,
                         m_elementType ? m_elementType->getDescription().c_str() : "(null)");
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

bool StructType::containByrefMember()
{
    for (StructMember *s : getMembers())
    {
        if (s->isByref())
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

string StructType::getDescription() const
{
    string members;
    unsigned int n = 0;
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

string StructMember::getDescription() const
{
    return format_string("<member %s:%s>", m_name.c_str(), (m_dataType ? m_dataType->getName().c_str() : "(no type)"));
}

EnumMember *EnumType::getMember(const string &name)
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

string EnumType::getDescription() const
{
    string members;
    unsigned int n = 0;
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

string EnumMember::getDescription() const
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

void Group::addInterface(Interface *iface)
{
    assert(iface);
    m_interfaces.push_back(iface);
}

void Group::addDirToSymbolsMap(Symbol *symbol, param_direction_t dir)
{
    Log::info("Adding direction %d for symbol \"%s\"\n", dir, symbol->getName().c_str());
    auto it = m_symbolDirections.find(symbol);
    if (it == m_symbolDirections.end())
    {
        set<param_direction_t> directions;
        directions.insert(dir);
        m_symbolDirections[symbol] = directions;

        // add symbol into list of symbols
        if (find(m_symbols.begin(), m_symbols.end(), symbol) == m_symbols.end())
        {
            m_symbols.push_back(symbol);
        }
        return;
    }

    it->second.insert(dir);
}

void Group::setTemplate(cpptempl::data_map groupTemplate)
{
    m_template = groupTemplate;
}

const set<param_direction_t> Group::getSymbolDirections(Symbol *symbol) const
{
    set<param_direction_t> directions;
    auto it = m_symbolDirections.find(symbol);
    if (it != m_symbolDirections.end())
    {
        directions = it->second;
    }

    return directions;
}

string Group::getDescription() const
{
    string ifaces;
    unsigned int n = 0;
    for (auto it : m_interfaces)
    {
        ifaces += format_string("%d:", n);
        ifaces += it->getDescription();
        if (n < m_interfaces.size() - 1)
        {
            ifaces += ", ";
        }
        ++n;
    }
    return format_string("<group \"%s\" [%s]>", m_name.c_str(), ifaces.c_str());
}

string AliasType::getDescription() const
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
        case DataType::data_type_t::kListType: {
            ListType *l = dynamic_cast<ListType *>(trueDataType);
            assert(l);
            return l->getElementType()->getTrueContainerDataType();
        }
        case DataType::data_type_t::kArrayType: {
            ArrayType *a = dynamic_cast<ArrayType *>(trueDataType);
            assert(a);
            return a->getElementType()->getTrueContainerDataType();
        }
        default:
            return trueDataType;
    }
}

string FunctionType::getDescription() const
{
    return format_string("<function %s->%s [%s]>", m_name.c_str(),
                         (m_returnType ? m_returnType->getDescription().c_str() : "(oneway)"),
                         m_parameters.getDescription().c_str());
}

string Function::getDescription() const
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

void Interface::addFunctionType(FunctionType *func)
{
    assert(func);

    m_scope.addSymbol(func);
    m_functionTypes.push_back(func);
}

string Interface::getDescription() const
{
    string fns;
    unsigned int n = 0;
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

string UnionCase::getDescription() const
{
    string description;
    string caseName = ("" != m_caseName) ? m_caseName : to_string(m_caseValue);
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

StructMember *UnionCase::getUnionMemberDeclaration(const string &name)
{
    return m_containingUnion->getUnionMemberDeclaration(name);
}

/* UnionType */

void UnionType::addCase(UnionCase *unionCase)
{
    unionCase->setUnionReferencePointer(this);
    m_unionCases.push_back(unionCase);
}

string UnionType::getDescription() const
{
    string description;
    for (auto caseMember : m_unionCases)
    {
        description += caseMember->getDescription();
    }
    return format_string("<union %s [%s]>", m_name.c_str(), description.c_str());
}

UnionType::case_vector_t UnionType::getUniqueCases()
{
    UnionType::case_vector_t uniqueCases;
    bool uniqueAddCase = true;
    for (auto unionCase : getCases())
    {
        for (auto uniqueCase : uniqueCases)
        {
            if (casesAreTheSame(unionCase, uniqueCase))
            {
                uniqueAddCase = false;
            }
        }
        if (uniqueAddCase)
        {
            uniqueCases.push_back(unionCase);
        }
        uniqueAddCase = true;
    }
    return uniqueCases;
}

bool UnionType::casesAreTheSame(UnionCase *a, UnionCase *b)
{
    vector<string> aNames = a->getMemberDeclarationNames();
    vector<string> bNames = b->getMemberDeclarationNames();
    if (aNames.size() != bNames.size())
    {
        return false;
    }
    for (unsigned int i = 0; i < aNames.size(); ++i)
    {
        if (aNames[i] != bNames[i])
        {
            return false;
        }
    }
    return true;
}

bool UnionType::addUnionMemberDeclaration(const string &name, DataType *dataType)
{
    if (declarationExists(name))
    {
        throw semantic_error(format_string("Redefinition of union member: '%s'\n", name.c_str()));
    }
    StructMember *newMember = new StructMember(name, dataType);
    m_members.addMember(newMember);
    return true;
}

bool UnionType::declarationExists(const string &name)
{
    for (auto member : m_members.getMembers())
    {
        if (name == member->getName())
        {
            return true;
        }
    }
    return false;
}

StructMember *UnionType::getUnionMemberDeclaration(const string &name)
{
    for (auto caseMember : m_members.getMembers())
    {
        if (name == caseMember->getName())
        {
            return caseMember;
        }
    }
    throw semantic_error(format_string("Union member not found: '%s'\n", name.c_str()));
}

void UnionType::printUnionMembers()
{
    for (auto member : m_members.getMembers())
    {
        Log::debug("Member declaration:%s %s\n", member->getDataType()->getName().c_str(), member->getName().c_str());
    }
}
