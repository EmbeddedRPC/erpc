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

#include "UniqueIdChecker.h"
#include "Symbol.h"
#include "Logging.h"
#include "Annotation.h"
#include "ParseErrors.h"
#include "annotations.h"

#include <sstream>
#include <cstdio>

using namespace erpcgen;
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

void UniqueIdChecker::makeIdsUnique(erpcgen::InterfaceDefinition &def)
{
    initUsedInterfaceIds(def.getGlobals().getSymbolsOfType(Symbol::kInterfaceSymbol));

    for (auto it : def.getGlobals().getSymbolsOfType(Symbol::kInterfaceSymbol))
    {
        assert(nullptr != it);
        Interface *interface = dynamic_cast<Interface *>(it);
        assert(interface);
        if (Annotation *interfaceId = interface->findAnnotation(ID_ANNOTATION))
        {
            setInterfaceId(interface, interfaceId);
        }
        initUsedFunctionIds(interface);
        for (auto function : interface->getFunctions())
        {
            if (Annotation *functionId = function->findAnnotation(ID_ANNOTATION))
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
        for (int i = 0; i < ifaces.size(); ++i)
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
        for (int i = 0; i < functions.size(); ++i)
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
        int newIdValue = dynamic_cast<IntegerValue *>(interfaceId->getValueObject())->getValue();
        if (0 >= newIdValue && (0 != iface->getName().compare("Common")))
        {
            throw semantic_error(
                format_string("@id value for interface %s must be greater than zero", iface->getName().c_str()));
        }
        iface->setUniqueId(newIdValue);
        for (int i = 0; i < m_usedInterfaceIds.size(); ++i)
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
        int newIdValue = dynamic_cast<IntegerValue *>(idAnnotation->getValueObject())->getValue();
        if (0 >= newIdValue)
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
        for (int i = 0; i < m_usedFunctionIds.size(); ++i)
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
        sort(ids.begin(), ids.end(), [](const idAndName_t &left, const idAndName_t &right)
             {
                 return left.first < right.first;
             });

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
    std::stringstream idStringStream;
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
