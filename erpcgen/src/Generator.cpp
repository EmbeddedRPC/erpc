/*
 * Copyright (c) 2014-2015, Freescale Semiconductor, Inc.
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

#include "Generator.h"
#include "Logging.h"
#include "annotations.h"
#include "format_string.h"
#include <boost/filesystem.hpp>
#include <ctime>
#include <string.h>

using namespace erpcgen;
using namespace cpptempl;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

Generator::Generator(InterfaceDefinition *def)
: m_def(def)
, m_globals(&(m_def->getGlobals()))
{
}

void Generator::openFile(std::ofstream &fileOutputStream, const std::string &fileName)
{
    boost::filesystem::path outputDir = m_def->getOutputDirectory();
    if (!outputDir.empty())
    {
        // TODO: do we have to create a copy of the outputDir here? Doesn't make sense...
        boost::filesystem::path dir(outputDir);
        if (!boost::filesystem::is_directory(dir))
        {
            // Create_directories function return false also when it create new directory.
            // It is in case, when directory ends with slash. For these case is better use is_directory for check if
            // directories are created.
            boost::filesystem::create_directories(dir);
            if (!boost::filesystem::is_directory(dir))
            {
                throw std::runtime_error(format_string("could not create directory path '%s'", outputDir.c_str()));
            }
        }
    }
    std::string filePathWithName = (outputDir / fileName).string();
    // Open file.
    fileOutputStream.open(filePathWithName, std::ios::out | std::ios::binary);
    if (!fileOutputStream.is_open())
    {
        throw std::runtime_error(format_string("could not open output file '%s'", filePathWithName.c_str()));
    }
}

void Generator::generateOutputFile(const std::string &fileName,
                                   const std::string &templateName,
                                   data_map &templateData,
                                   const char *const kParseFile)
{
    std::ofstream fileOutputStream;
    openFile(fileOutputStream, fileName);

    // Run template and write output to output files. Catch and rethrow template exceptions
    // so we can add the name of the template that caused the error to aid in debugging.
    try
    {
        parse(fileOutputStream, kParseFile, templateData);
        fileOutputStream.close();
    }
    catch (TemplateException &e)
    {
        throw TemplateException(format_string("Template %s: %s", templateName.c_str(), e.what()));
    }
}

std::string Generator::stripExtension(const std::string &filename)
{
    auto result = filename.rfind('.');
    if (result != std::string::npos)
    {
        return filename.substr(0, result);
    }
    else
    {
        return filename;
    }
}

StructMember *Generator::findParamReferencedFrom(const StructType::member_vector_t &members,
                                                 const std::string &referenceName,
                                                 const std::string &annName)
{
    for (StructMember *structMember : members)
    {
        Annotation *ann = structMember->findAnnotation(annName);
        if (ann)
        {
            std::string lengthName = ann->getValueObject()->toString();
            if (strcmp(lengthName.c_str(), referenceName.c_str()) == 0)
            {
                return structMember;
            }
        }
    }
    return nullptr;
}

std::string Generator::getTime()
{
    std::time_t now = std::time(nullptr);
    std::string nowString = std::ctime(&now);
    nowString.pop_back(); // Remove trailing newline.
    return nowString;
}

Generator::interfaceLists_t Generator::makeInterfacesTemplateData()
{
    Log::info("interfaces:\n");
    int n = 0;
    interfaceLists_t interfaceLists;
    for (auto it : m_globals->getSymbolsOfType(Symbol::kInterfaceSymbol))
    {
        Interface *iface = dynamic_cast<Interface *>(it);
        assert(iface);
        data_map ifaceInfo;
        ifaceInfo["name"] = make_data(iface->getName());
        ifaceInfo["id"] = data_ptr(iface->getUniqueId());

        getInterfaceComments(iface, ifaceInfo);

        // TODO: for C only?
        ifaceInfo["serviceClassName"] = iface->getName() + "_service";

        Log::info("%d: (%d) %s\n", n, iface->getUniqueId(), iface->getName().c_str());

        ifaceInfo["functions"] = getFunctionsTemplateData(iface);
        ++n;

        // Sorting interfaces into groups.
        if (it->findAnnotation(GROUP_ANNOTATION))
        {
            for (auto group : it->getAnnotations(GROUP_ANNOTATION))
            {
                std::string groupName = (group->hasValue()) ? group->getValueObject()->toString() : "";
                fillInterfaceListsWithMap(interfaceLists, ifaceInfo, groupName);
            }
        }
        else
        {
            fillInterfaceListsWithMap(interfaceLists, ifaceInfo, "");
        }
    }
    return interfaceLists;
}

data_list Generator::getFunctionsTemplateData(Interface *iface)
{
    data_list fns;
    int j = 0;
    for (auto fit : iface->getFunctions())
    {
        fns.push_back(make_data(getFunctionTemplateData(fit, j++)));
    }
    return fns;
}

void Generator::fillInterfaceListsWithMap(interfaceLists_t &interfaceLists,
                                          cpptempl::data_ptr interfaceMap,
                                          std::string mapName)
{
    auto interfaceList = interfaceLists.find(mapName);
    if (interfaceList == interfaceLists.end())
    {
        std::vector<cpptempl::data_ptr> interfaceVector;
        interfaceLists[mapName] = interfaceVector;
        interfaceList = interfaceLists.find(mapName);
    }
    interfaceList->second.push_back(interfaceMap);
}

void Generator::generateInterfaceOutputFiles(cpptempl::data_map &templateData, interfaceLists_t interfaceLists)
{
    // Generate output files. We have to special case not having any interfaces define.
    if (interfaceLists.empty())
    {
        // empty list of interfaces
        data_list empty;
        templateData["interfaces"] = empty;

        // Log template data.
        if (Log::getLogger()->getFilterLevel() >= Logger::kDebug2)
        {
            dump_data(templateData);
        }

        generateOutputFiles("");
    }
    else
    {
        for (auto interfaceList : interfaceLists)
        {
            templateData["interfaces"] = interfaceList.second;

            // Log template data.
            if (Log::getLogger()->getFilterLevel() >= Logger::kDebug2)
            {
                dump_data(templateData);
            }

            generateOutputFiles(interfaceList.first);
        }
    }
}

void Generator::makeIncludesTemplateData(cpptempl::data_map &templateData)
{
    data_list includeData;
    if (m_def->hasProgramSymbol())
    {
        for (auto include : m_def->programSymbol()->getAnnotations(INCLUDE_ANNOTATION))
        {
            includeData.push_back(make_data(include->getValueObject()->toString()));
            Log::info("include %s\n", include->getValueObject()->toString().c_str());
        }
    }
    templateData["includes"] = includeData;
}
