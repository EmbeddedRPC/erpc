/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "UniqueIdChecker.hpp"

#include "Annotation.hpp"
#include "Logging.hpp"
#include "ParseErrors.hpp"
#include "Symbol.hpp"
#include "annotations.h"

#include <algorithm>
#include <cstdio>
#include <sstream>

using namespace erpcgen;
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

void UniqueIdChecker::makeIdsUnique(InterfaceDefinition &def)
{
    initUsedInterfaceIds(def.getGlobals().getSymbolsOfType(Symbol::symbol_type_t::kInterfaceSymbol));

    for (auto it : def.getGlobals().getSymbolsOfType(Symbol::symbol_type_t::kInterfaceSymbol))
    {
        assert(nullptr != it);
        Interface *interface = dynamic_cast<Interface *>(it);
        assert(interface);
        if (Annotation *interfaceId = interface->findAnnotation(ID_ANNOTATION, Annotation::program_lang_t::kAll))
        {
            setInterfaceId(interface, interfaceId);
        }
        initUsedFunctionIds(interface);
        for (auto function : interface->getFunctions())
        {
            if (Annotation *functionId = function->findAnnotation(ID_ANNOTATION, Annotation::program_lang_t::kAll))
            {
                setFunctionId(function, functionId);
            }
        }
        if (0 < m_usedFunctionIds.size())
        {
            checkDuplicateIds(m_usedFunctionIds, "function");
        }
    }
    if (0 < m_usedInterfaceIds.size())
    {
        checkDuplicateIds(m_usedInterfaceIds, "interface");
    }
}

void UniqueIdChecker::initUsedInterfaceIds(SymbolScope::symbol_vector_t ifaces)
{
    if (0 < ifaces.size())
    {
        for (unsigned int i = 0; i < ifaces.size(); ++i)
        {
            Interface *iface = dynamic_cast<Interface *>(ifaces[i]);
            assert(iface);
            m_usedInterfaceIds.push_back(make_pair(iface->getUniqueId(), iface->getName()));
        }
    }
}

void UniqueIdChecker::initUsedFunctionIds(Interface *iface)
{
    m_usedFunctionIds.clear();
    Interface::function_vector_t functions = iface->getFunctions();
    if (0 < functions.size())
    {
        for (unsigned int i = 0; i < functions.size(); ++i)
        {
            m_usedFunctionIds.push_back(make_pair(functions[i]->getUniqueId(), functions[i]->getName()));
        }
    }
}

void UniqueIdChecker::setInterfaceId(Interface *iface, Annotation *interfaceId)
{
    if (!interfaceId->hasValue() || interfaceId->getValueObject()->getType() != kIntegerValue)
    {
        throw semantic_error(format_string("@id() for interface %s() has no valid value\n", iface->getName().c_str()));
    }
    else
    {
        assert(nullptr != dynamic_cast<IntegerValue *>(interfaceId->getValueObject()));
        uint32_t newIdValue = (uint32_t) dynamic_cast<IntegerValue *>(interfaceId->getValueObject())->getValue();
        if (newIdValue == 0 && 0 != iface->getName().compare("Common"))
        {
            throw semantic_error(
                format_string("@id value for interface %s must be greater than zero", iface->getName().c_str()));
        }
        iface->setUniqueId(newIdValue);
        for (unsigned int i = 0; i < m_usedInterfaceIds.size(); ++i)
        {
            if (0 == m_usedInterfaceIds[i].second.compare(iface->getName()))
            {
                m_usedInterfaceIds.erase(m_usedInterfaceIds.begin() + i);
                break;
            }
        }
        m_usedInterfaceIds.push_back(make_pair(newIdValue, iface->getName()));
    }
}

void UniqueIdChecker::setFunctionId(Function *fn, Annotation *idAnnotation)
{
    if (!idAnnotation->hasValue())
    {
        throw semantic_error(format_string("@id() for function %s() has no valid value\n", fn->getName().c_str()));
    }
    else
    {
        assert(nullptr != dynamic_cast<IntegerValue *>(idAnnotation->getValueObject()));
        uint32_t newIdValue = (uint32_t) dynamic_cast<IntegerValue *>(idAnnotation->getValueObject())->getValue();
        if (0 == newIdValue)
        {
            throw semantic_error(
                format_string("@id value for function %s must be greater than zero", fn->getName().c_str()));
        }
        fn->setUniqueId(newIdValue);
        /*
        for(int i=0; i < usedFunctionIds.size(); ++i)
        {
            printf("%d: <id: %d, name: %s>\n",i, usedFunctionIds[i].first, usedFunctionIds[i].second.c_str());
        }
        */
        for (unsigned int i = 0; i < m_usedFunctionIds.size(); ++i)
        {
            //    printf("usedFunctionIds at i: %s\t", usedFunctionIds[i].second.c_str());
            //   printf("fn name: %s\n", fn->getName().c_str());
            if (0 == m_usedFunctionIds[i].second.compare(fn->getName()))
            {
                m_usedFunctionIds.erase(m_usedFunctionIds.begin() + i);
                break;
            }
        }
        m_usedFunctionIds.push_back(make_pair(newIdValue, fn->getName()));
    }
}

void UniqueIdChecker::checkDuplicateIds(vector<idAndName_t> ids, string idType)
{
    set<idAndName_t> duplicateIdsAndNames;
    set<int> tempIdHolder;
    if (1 < ids.size())
    {
        sort(ids.begin(), ids.end(),
             [](const idAndName_t &left, const idAndName_t &right) { return left.first < right.first; });

        vector<idAndName_t>::iterator it;
        for (it = ids.begin(); it != ids.end() - 1; ++it)
        {
            if (it->first == (it + 1)->first)
            {
                duplicateIdsAndNames.insert(*it);
                duplicateIdsAndNames.insert(*(it + 1));
            }
        }

        if (0 < duplicateIdsAndNames.size())
        {
            printDuplicateIdWarning(duplicateIdsAndNames, idType);
        }
    }
}

void UniqueIdChecker::printDuplicateIdWarning(set<idAndName_t> duplicateIds, string idType)
{
    stringstream idStringStream;
    int currentDuplicateId = duplicateIds.begin()->first;
    for (idAndName_t duplicatePair : duplicateIds)
    {
        int id = duplicatePair.first;
        if (currentDuplicateId != id)
        {
            idStringStream << endl;
            currentDuplicateId = id;
        }
        string name = duplicatePair.second;
        idStringStream << "<id: " << id << ", " << idType << ": " << name << "> ";
    }
    Log::warning("The following %s ids are not unique, and will cause the generated code to not compile:\n",
                 idType.c_str());
    Log::warning("%s\n", idStringStream.str().c_str());
}
