/*
 * Copyright (c) 2014-2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
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
#include "ParseErrors.h"
#include "annotations.h"
#include "erpcgen_version.h"
#include "format_string.h"
#include <boost/filesystem.hpp>
#include <cstring>
#include <ctime>

using namespace erpcgen;
using namespace cpptempl;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

Generator::Generator(InterfaceDefinition *def, uint16_t idlCrc16)
: m_def(def)
, m_globals(&(m_def->getGlobals()))
, m_idlCrc16(idlCrc16)
{
    m_templateData["erpcgenVersion"] = ERPCGEN_VERSION;

    // crc of erpcgen version and idl files.
    m_templateData["crc16"] = m_idlCrc16;

    m_templateData["todaysDate"] = getTime();

    if (m_def->hasProgramSymbol())
    {
        Log::info("program: ");
        Log::info("%s\n", m_def->getOutputFilename().c_str());

        /* Shared memory area. */
        Annotation *sharedMemBAnn = def->programSymbol()->findAnnotation(SHARED_MEMORY_BEGIN_ANNOTATION);
        Annotation *sharedMemEAnn = def->programSymbol()->findAnnotation(SHARED_MEMORY_BEGIN_ANNOTATION);
        if (sharedMemBAnn && sharedMemEAnn)
        {
            Value *sharedMemAValue = sharedMemBAnn->getValueObject();
            Value *sharedMemEValue = sharedMemEAnn->getValueObject();
            if (sharedMemAValue && sharedMemEValue)
            {
                m_templateData["sharedMemBeginAddr"] = sharedMemAValue->toString();
                m_templateData["sharedMemEndAddr"] = sharedMemEValue->toString();
                Log::warning("Shared memory is supported only for C language used on embedded devices.\n");
            }
            else
            {
                throw semantic_error("Annotations @shared_memory_begin and @shared_memory_end need contains addresses.");
            }
        }
        else if (sharedMemBAnn || sharedMemEAnn)
        {
            throw semantic_error("Need be defined both or no one of shared memory regions annotations through @shared_memory_begin and @shared_memory_end.");
        }
        else
        {
            m_templateData["sharedMemBeginAddr"] = "";
            m_templateData["sharedMemEndAddr"] = "";
        }
    }
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

StructMember *Generator::findParamReferencedFromAnn(const StructType::member_vector_t &members,
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

StructMember *Generator::findParamReferencedFromUnion(const StructType::member_vector_t &members,
                                                      const std::string &referenceName)
{
    for (StructMember *structMember : members)
    {
        DataType *trueDataType = structMember->getDataType()->getTrueDataType();
        if (trueDataType->isUnion())
        {
            UnionType *unionType = dynamic_cast<UnionType *>(trueDataType);
            if (unionType->isNonEncapsulatedUnion())
            {
                Annotation *ann = structMember->findAnnotation(DISCRIMINATOR_ANNOTATION);
                if (ann)
                {
                    std::string lengthName = ann->getValueObject()->toString();
                    if (strcmp(lengthName.c_str(), referenceName.c_str()) == 0)
                    {
                        return structMember;
                    }
                }
            }
            else
            {
                if (strcmp(unionType->getDiscriminatorName().c_str(), referenceName.c_str()) == 0)
                {
                    return structMember;
                }
            }
        }
    }
    return nullptr;
}

StructMember *Generator::findParamReferencedFrom(const StructType::member_vector_t &members,
                                                 const std::string &referenceName)
{
    StructMember *referencedFrom = findParamReferencedFromAnn(members, referenceName, LENGTH_ANNOTATION);

    if (referencedFrom)
    {
        return referencedFrom;
    }
    else
    {
        return findParamReferencedFromUnion(members, referenceName);
    }
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
        ifaceInfo["name"] = make_data(iface->getOutputName());
        ifaceInfo["id"] = data_ptr(iface->getUniqueId());

        setTemplateComments(iface, ifaceInfo);

        // TODO: for C only?
        ifaceInfo["serviceClassName"] = iface->getOutputName() + "_service";

        Log::info("%d: (%d) %s\n", n, iface->getUniqueId(), iface->getName().c_str());

        /* Has interface function declared as non-external? */
        data_list functions = getFunctionsTemplateData(iface);
        ifaceInfo["functions"] = functions;
        ifaceInfo["isNonExternalInterface"] = false;
        for (int i = 0; i < functions.size(); ++i)
        {
            std::string isNonExternalFunction = dynamic_cast<DataMap *>(functions[i].get().get())->getmap()["isNonExternalFunction"]->getvalue();
            if (isNonExternalFunction.compare("true") == 0)
            {
                ifaceInfo["isNonExternalInterface"] = true;
                break;
            }
        }
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
    templateData["groupClientC"] = "";  //when interface is not presented
    templateData["groupClientPY"] = ""; //when interface is not presented

    /* Generate file containing crc of IDL files. */
    generateCrcFile();

    // Generate output files. We have to special case not having any interfaces define.
    if (interfaceLists.empty())
    {
        // empty list of interfaces
        data_list empty;
        templateData["group"] = "";
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
        /* TODO: temporary C/PY code. */
        std::vector<std::string> groupNames;
        if (m_globals->getSymbolsOfType(Symbol::kFunctionTypeSymbol).size() > 0)
        {
            for (auto interfaceList : interfaceLists)
            {
                groupNames.push_back(interfaceList.first);
            }
        }

        for (auto interfaceList : interfaceLists)
        {
            /* TODO: temporary C/PY code. */
            std::string cName;
            std::string pyName;
            for (std::string groupName : groupNames)
            {
                if (groupName != interfaceList.first)
                {
                    cName += "#include \"" + stripExtension(m_def->getOutputFilename()) + "_" + groupName + ".h\"\n";
                    pyName += "from ../" + stripExtension(m_def->getOutputFilename()) + "_" + groupName + " import interface\n";
                }
            }

            templateData["groupClientC"] = cName;
            templateData["groupClientPY"] = pyName;
            templateData["group"] = interfaceList.first;
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

    /* find server includes */
    std::map<std::string, std::vector<std::string> > groupIncludes;
    for (Symbol *interfaceSymbol : m_globals->getSymbolsOfType(Symbol::kInterfaceSymbol))
    {
        Interface *interface = dynamic_cast<Interface *>(interfaceSymbol);
        assert(interface);
        Annotation *includeAnn = interface->findAnnotation(INCLUDE_ANNOTATION);
        if (includeAnn)
        {
            Annotation *groupAnn = interface->findAnnotation(GROUP_ANNOTATION);
            std::string groupName;
            if (groupAnn)
            {
                groupName = groupAnn->getValueObject()->toString();
            }
            else
            {
                groupName = "";
            }
            auto includeList = groupIncludes.find(groupName);
            if (includeList == groupIncludes.end())
            {
                std::vector<std::string> includeVector;
                groupIncludes[groupName] = includeVector;
                includeList = groupIncludes.find(groupName);
            }
            includeList->second.push_back(includeAnn->getValueObject()->toString());
        }
    }

    /* fill templateData server includes */
    data_list groupsIncludes;
    for (auto includeList : groupIncludes)
    {
        data_list groupIncludesList;
        data_map groupInfo;
        groupInfo["name"] = includeList.first;
        for (auto includeName : includeList.second)
        {
            groupIncludesList.push_back(includeName);
        }
        groupInfo["includes"] = groupIncludesList;
        groupsIncludes.push_back(groupInfo);
    }
    templateData["groupsServer"] = groupsIncludes;
}
