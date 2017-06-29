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
#include "erpc_version.h"
#include "Logging.h"
#include "ParseErrors.h"
#include "annotations.h"
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
    m_templateData["erpcVersion"] = ERPC_VERSION;
    m_templateData["erpcVersionNumber"] = ERPC_VERSION_NUMBER;

    // crc of erpcgen version and idl files.
    m_templateData["crc16"] = m_idlCrc16;

    m_templateData["todaysDate"] = getTime();

    m_templateData["sharedMemBeginAddr"] = "";
    m_templateData["sharedMemEndAddr"] = "";

    if (m_def->hasProgramSymbol())
    {
        Log::info("program: ");
        Log::info("%s\n", m_def->getOutputFilename().c_str());

        Program *program = m_def->getProgramSymbol();

        /* Shared memory area. */
        Value *sharedMemBValue = program->getAnnValue(SHARED_MEMORY_BEGIN_ANNOTATION);
        Value *sharedMemEValue = program->getAnnValue(SHARED_MEMORY_END_ANNOTATION);
        if (sharedMemBValue && sharedMemEValue)
        {
            m_templateData["sharedMemBeginAddr"] = sharedMemBValue->toString();
            m_templateData["sharedMemEndAddr"] = sharedMemEValue->toString();
            Log::warning("Shared memory is supported only for C language used on embedded devices.\n");
        }
        else if (sharedMemBValue || sharedMemEValue)
        {
            throw semantic_error("Annotations @shared_memory_begin and @shared_memory_end both (or no one) need exists and contains addresses.");
        }
    }

    // get group annotation with vector of theirs interfaces
    m_groups.clear();
    data_list groupNames;
    Group *defaultGroup = new Group("");

    for (auto it : m_globals->getSymbolsOfType(Symbol::kInterfaceSymbol))
    {
        Interface *iface = dynamic_cast<Interface *>(it);
        assert(iface);

        // interface has group annotation
        std::vector<Annotation *> groupAnns = iface->getAnnotations(GROUP_ANNOTATION);
        if (!groupAnns.empty())
        {
            for (auto groupAnnIt : groupAnns)
            {
                std::string name = (groupAnnIt->hasValue()) ? groupAnnIt->getValueObject()->toString() : "";
                Group *group = getGroupByName(name);
                if (group == nullptr)
                {
                    group = new Group(name);
                    m_groups.push_back(group);
                    groupNames.push_back(name);
                }

                group->addInterface(iface);
            }
        }
        else
        {
            // interface belongs to default group
            defaultGroup->addInterface(iface);
        }
    }

    // add default group only if it has any interface or there is no other group
    if (defaultGroup->getInterfaces().size() > 0 || m_groups.size() == 0)
    {
        m_groups.push_back(defaultGroup);
    }

    Log::debug("Groups:\n");
    for (Group *group : m_groups)
    {
        Log::log("    %s\n", group->getDescription().c_str());
    }

    // list of group names (used for including group header files for callbacks)
    m_templateData["groupNames"] = groupNames;
}

Group *Generator::getGroupByName(std::string name)
{
    for (Group *group : m_groups)
    {
        if (group->getName() == name)
        {
            return group;
        }
    }

    return nullptr;
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
        std::string lengthName = structMember->getAnnStringValue(annName);
        if (strcmp(lengthName.c_str(), referenceName.c_str()) == 0)
        {
            return structMember;
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
                std::string lengthName = structMember->getAnnStringValue(DISCRIMINATOR_ANNOTATION);
                if (strcmp(lengthName.c_str(), referenceName.c_str()) == 0)
                {
                    return structMember;
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

DataType *Generator::findChildDataType(std::vector<DataType *> *dataTypes, DataType *dataType)
{
    switch (dataType->getDataType())
    {
        case DataType::kAliasType:
        {
            AliasType *aliasType = dynamic_cast<AliasType *>(dataType);
            if (aliasType != nullptr)
            {
                findChildDataType(dataTypes, aliasType->getElementType());
            }
            break;
        }
        case DataType::kArrayType:
        {
            ArrayType *arrayType = dynamic_cast<ArrayType *>(dataType);
            if (arrayType != nullptr)
            {
                findChildDataType(dataTypes, arrayType->getElementType());
            }
            break;
        }
        case DataType::kBuiltinType:
        {
            if (dataType->isBinary())
            {
                DataType *binaryDataType = dynamic_cast<DataType *>(m_globals->getSymbol("binary_t"));
                if (binaryDataType != nullptr)
                {
                    dataType = binaryDataType;
                }
            }
            break;
        }
        case DataType::kListType:
        {
            ListType *listType = dynamic_cast<ListType *>(dataType);
            if (listType != nullptr)
            {
                findChildDataType(dataTypes, listType->getElementType());
            }
            break;
        }
        case DataType::kStructType:
        {
            StructType *structType = dynamic_cast<StructType *>(dataType);
            if (structType != nullptr)
            {
                for (StructMember *structMember : structType->getMembers())
                {
                    findChildDataType(dataTypes, structMember->getDataType());
                }
            }
            break;
        }
        case DataType::kUnionType:
        {
            // Keil need extra pragma option when unions are used.
            m_templateData["usedUnionType"] = true;
            UnionType *unionType = dynamic_cast<UnionType *>(dataType);
            if (unionType != nullptr)
            {
                for (auto &unionMember : unionType->getUnionMemberDeclarations())
                {
                    findChildDataType(dataTypes, unionMember.getDataType());
                }
            }
            break;
        }
        default:
        {
            break;
        }
    }

    dataTypes->push_back(dataType);

    return dataType;
}

void Generator::findGroupDataTypes()
{
    for (Group *group : m_groups)
    {
        for (Interface *iface : group->getInterfaces())
        {
            for (Function *fn : iface->getFunctions())
            {
                // handle return value
                std::vector<DataType *> dataTypes;
                DataType *transformedDataType = findChildDataType(&dataTypes, fn->getReturnType());
                fn->setReturnType(transformedDataType);

                // save all transformed data types directions into data type map
                if (!fn->getReturnType()->findAnnotation(SHARED_ANNOTATION))
                {
                    for (DataType *dataType : dataTypes)
                    {
                        group->addDirToSymbolsMap(dataType, kReturn);
                    }
                }

                // handle function parameters
                auto params = fn->getParameters().getMembers();
                for (auto mit : params)
                {
                    dataTypes.clear();

                    setBinaryList(mit);

                    mit->setDataType(findChildDataType(&dataTypes, mit->getDataType()));

                    // save all transformed data types directions into data type map
                    if (!mit->findAnnotation(SHARED_ANNOTATION))
                    {
                        for (DataType *dataType : dataTypes)
                        {
                            group->addDirToSymbolsMap(dataType, mit->getDirection());
                        }
                    }
                }
            }
        }
    }
}

data_list Generator::makeGroupInterfacesTemplateData(Group *group)
{
    Log::info("interfaces:\n");
    data_list interfaces;
    int n = 0;

    for (Interface *iface : group->getInterfaces())
    {
        data_map ifaceInfo;
        ifaceInfo["name"] = make_data(iface->getOutputName());
        ifaceInfo["id"] = data_ptr(iface->getUniqueId());

        setTemplateComments(iface, ifaceInfo);

        // TODO: for C only?
        ifaceInfo["serviceClassName"] = iface->getOutputName() + "_service";

        Log::info("%d: (%d) %s\n", n++, iface->getUniqueId(), iface->getName().c_str());

        /* Has interface function declared as non-external? */
        data_list functions = getFunctionsTemplateData(group, iface);
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

        interfaces.push_back(ifaceInfo);
    }

    return interfaces;
}

data_list Generator::getFunctionsTemplateData(Group *group, Interface *iface)
{
    data_list fns;

    int j = 0;
    for (auto fit : iface->getFunctions())
    {
        fns.push_back(make_data(getFunctionTemplateData(group, fit, j++)));
    }
    return fns;
}

void Generator::generateGroupOutputFiles(Group *group)
{
    // generate output files only for groups with interfaces or for IDLs with no interfaces at all
    if (!group->getInterfaces().empty() || (m_groups.size() == 1 && group->getName() == ""))
    {
        std::string groupName = group->getName();
        std::string fileName = stripExtension(m_def->getOutputFilename());
        m_templateData["outputFilename"] = fileName;
        if (groupName != "")
        {
            fileName += "_" + groupName;
        }
        Log::info("File name %s\n", fileName.c_str());
        m_templateData["commonHeaderName"] = fileName;

        // group templates
        m_templateData["group"] = group->getTemplate();

        // Log template data.
        if (Log::getLogger()->getFilterLevel() >= Logger::kDebug2)
        {
            dump_data(m_templateData);
        }

        generateOutputFiles(fileName);
    }
}

void Generator::makeIncludesTemplateData()
{
    data_list includeData;
    if (m_def->hasProgramSymbol())
    {
        for (auto include : m_def->getProgramSymbol()->getAnnotations(INCLUDE_ANNOTATION))
        {
            includeData.push_back(make_data(include->getValueObject()->toString()));
            Log::info("include %s\n", include->getValueObject()->toString().c_str());
        }
    }
    m_templateData["includes"] = includeData;
}

data_list Generator::makeGroupIncludesTemplateData(Group *group)
{
    data_list includes;
    std::set<std::string> tempSet;

    for (Interface *iface : group->getInterfaces())
    {
        Annotation *includeAnn = iface->findAnnotation(INCLUDE_ANNOTATION);
        if (includeAnn)
        {
            std::string include = includeAnn->getValueObject()->toString();
            if (tempSet.find(include) == tempSet.end())
            {
                includes.push_back(include);
                tempSet.insert(include);
            }
        }

        for (Function *func : iface->getFunctions())
        {
            Annotation *funcAnn = func->findAnnotation(INCLUDE_ANNOTATION);
            if (funcAnn)
            {
                std::string include = funcAnn->getValueObject()->toString();
                if (tempSet.find(include) == tempSet.end())
                {
                    includes.push_back(include);
                    tempSet.insert(include);
                }
            }
        }
    }

    return includes;
}
