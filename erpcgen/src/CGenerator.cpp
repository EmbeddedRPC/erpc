/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
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

#include "CGenerator.h"
#include "format_string.h"
#include "Logging.h"
#include "ParseErrors.h"
#include "erpcgen_version.h"
#include "annotations.h"
#include <algorithm>
#include <set>
#include <sstream>
#include <ctime>

using namespace erpcgen;
using namespace cpptempl;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

/*! @brief Set of characters that are allowed in C language identifiers. */
static const char *const kIdentifierChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";

// Templates strings converted from text files by txt_to_c.py.
extern const char *const kCCommonHeader;
extern const char *const kCServerHeader;
extern const char *const kCClientSource;
extern const char *const kCServerSource;
extern const char *const kCCoders;
extern const char *const kCCommonFunctions;

static std::vector<ListType *> s_binaryTypes;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

void CGenerator::generateClientServerOutputFiles(const std::string &fileNameExtension)
{
    std::string fileName = stripExtension(m_def->getOutputFilename());
    if (fileNameExtension != "")
    {
        fileName += "_" + fileNameExtension;
    }

    generateCommonHeaderFile(fileName);

    if (m_generateClientOutput)
    {
        generateClientSourceFile(fileName);
    }

    if (m_generateServerOutput)
    {
        generateServerHeaderFile(fileName);

        generateServerSourceFile(fileName);
    }
}

void CGenerator::generateCommonHeaderFile(std::string fileName)
{
    fileName += ".h";
    m_templateData["commonGuardMacro"] = generateIncludeGuardName(fileName);
    m_templateData["commonHeaderName"] = fileName;
    generateOutputFile(fileName, "c_common_header", m_templateData, kCCommonHeader);
}

void CGenerator::generateClientSourceFile(std::string fileName)
{
    m_templateData["source"] = "client";
    fileName += "_client.cpp";
    m_templateData["clientSourceName"] = fileName;
    generateOutputFile(fileName, "c_client_source", m_templateData, kCClientSource);
}

void CGenerator::generateServerHeaderFile(std::string fileName)
{
    fileName += "_server.h";
    m_templateData["serverGuardMacro"] = generateIncludeGuardName(fileName);
    m_templateData["serverHeaderName"] = fileName;
    generateOutputFile(fileName, "c_server_header", m_templateData, kCServerHeader);
}

void CGenerator::generateServerSourceFile(std::string fileName)
{
    m_templateData["source"] = "server";
    fileName += "_server.cpp";
    m_templateData["serverSourceName"] = fileName;
    generateOutputFile(fileName, "c_server_source", m_templateData, kCServerSource);
}

void CGenerator::parseSubtemplates()
{
    std::string templateName = "c_coders";
    try
    {
        parse(kCCoders, m_templateData);
        parse(kCCommonFunctions, m_templateData);
    }
    catch (TemplateException &e)
    {
        throw TemplateException(format_string("Template %s: %s", templateName.c_str(), e.what()));
    }
    templateName = "c_common_functions";
    try
    {
        parse(kCCoders, m_templateData);
        parse(kCCommonFunctions, m_templateData);
    }
    catch (TemplateException &e)
    {
        throw TemplateException(format_string("Template %s: %s", templateName.c_str(), e.what()));
    }
}

void CGenerator::transformBinaryDataType()
{
    // Replace binary with list<uint8>
    BuiltinType *builtinType = dynamic_cast<BuiltinType *>(m_globals->getSymbol("uint8"));
    assert(builtinType);
    ListType *listType = new ListType(builtinType);
    BuiltinType *replacedBuiltinType = dynamic_cast<BuiltinType *>(m_globals->getSymbol("binary"));
    assert(replacedBuiltinType);

    StructType *newStruct = new StructType("binary_t");
    StructMember *elements = new StructMember("data", listType);
    elements->setContainList(true);
    elements->setContainString(false);
    newStruct->addMember(elements);
    newStruct->getScope().setParent(m_globals);

    // Type definition for created structure
    AliasType *type = new AliasType("binary_t", newStruct);
    int32_t symbolPos = m_globals->getSymbolPos(replacedBuiltinType);
    m_globals->replaceSymbol(replacedBuiltinType, type);
    m_globals->addSymbol(newStruct, ++symbolPos);
    s_binaryTypes.push_back(listType);
}

DataType *CGenerator::transformDataType(DataType *dataType, _param_direction direction, DataType *topDataType)
{
    switch (dataType->getDataType())
    {
        case DataType::kBuiltinType:
        {
            if (dataType->isBinary())
            {
                dataType = dynamic_cast<DataType *>(m_globals->getSymbol("binary_t"));
                if (!dataType)
                {
                    transformBinaryDataType();
                    dataType = dynamic_cast<DataType *>(m_globals->getSymbol("binary_t"));
                    assert(dataType);
                }
                StructType *structType = dynamic_cast<StructType *>(dataType);
                assert(structType);
                if (direction != kNone)
                {
                    structType->addStructDirectionType(direction);
                }
            }
            break;
        }
        case DataType::kListType:
        {
            // The only child node of a list node is the element type.
            ListType *listType = dynamic_cast<ListType *>(dataType);
            assert(listType);
            DataType *elementType = transformDataType(listType->getElementType(), direction, topDataType);

            // If the list has a length variable, we do need to create a list struct.
            // Instead, we leave the list data type as is, and use that information
            // to generate template data.
            if (listType->hasLengthVariable())
            {
                Log::debug("list of type %s has length variable %s\n", listType->getElementType()->getName().c_str(),
                           listType->getLengthVariableName().c_str());
                break;
            }
            else
            {
                listType->setElementType(elementType);

                // Check if list already exist. If yes then use existing list. We need it for generating only one
                // send/received method.
                uint32_t nameCount = 0;

                std::string structName = format_string("list_%d_t", nameCount);
                Symbol *symbol = m_globals->getSymbol(structName);
                while (symbol != nullptr)
                {
                    DataType *symDataType = dynamic_cast<DataType *>(symbol);
                    assert(symDataType);
                    if (symDataType->getTrueDataType()->isStruct())
                    {
                        StructType *structType = dynamic_cast<StructType *>(symDataType);
                        assert(structType);
                        // For sure that structure hasn't zero members. Also this type of structure has only one member.
                        if (isListStruct(structType))
                        {
                            // use same structs to send same lists data types (including lists using alias name)
                            ListType *oldlistType =
                                dynamic_cast<ListType *>(structType->getMembers()[0]->getDataType());
                            assert(oldlistType);
                            if (oldlistType->getElementType()->getTrueDataType()->getName() ==
                                elementType->getTrueDataType()->getName())
                            {
                                if (direction != kNone)
                                {
                                    structType->addStructDirectionType(direction);
                                }
                                return symDataType;
                            }
                        }
                    }
                    ++nameCount;
                    structName = format_string("list_%d_t", nameCount);
                    symbol = m_globals->getSymbol(structName);
                }

                // if don't, create new one.
                StructType *newStruct = new StructType(structName);
                StructMember *elements = new StructMember("elements", listType);
                elements->setContainList(true);
                elements->setContainString(containsString(elementType));
                newStruct->addMember(elements);
                newStruct->getScope().setParent(m_globals);
                if (direction != kNone)
                {
                    newStruct->addStructDirectionType(direction);
                }

                AliasType *type = new AliasType(structName, newStruct);
                int32_t symbolPos = m_globals->getSymbolPos(topDataType);
                m_globals->addSymbol(type, symbolPos);
                if (symbolPos >= 0)
                {
                    ++symbolPos;
                }
                m_globals->addSymbol(newStruct, symbolPos);

                return newStruct;
            }
        }
        case DataType::kArrayType:
        {
            ArrayType *arrayType = dynamic_cast<ArrayType *>(dataType);
            assert(arrayType);
            arrayType->setElementType(transformDataType(arrayType->getElementType(), direction, topDataType));
            break;
        }
        case DataType::kStructType:
        {
            StructType *structType = dynamic_cast<StructType *>(dataType);
            assert(structType);
            if (direction != kNone)
            {
                structType->addStructDirectionType(direction);
            }

            StructMember *structMember = structType->getMembers()[0];
            // Todo: if list name will be changed to list_<type>_<number>_t this can be maybe optimized and more
            // precise.
            if (isListStruct(structType))
            {
                /* Not necessary catch return value. Should be same (already transformed). This is only because of
                 adding direction type. */
                transformDataType(structMember->getDataType()->getTrueContainerDataType(), direction, topDataType);
                return dataType;
            }

            for (StructMember *structMember : structType->getMembers())
            {
                setListLength(structMember);
                if (DataType::kUnionType == structMember->getDataType()->getDataType())
                {
                    // Keil need extra pragma option when unions are used.
                    m_templateData["usedUnionType"] = true;
                    UnionType *currentUnion = dynamic_cast<UnionType *>(structMember->getDataType());
                    assert(currentUnion);
                    for (auto &unionMember : currentUnion->getUnionMemberDeclarations())
                    {
                        setListLength(&unionMember);
                        unionMember.setDataType(transformDataType(unionMember.getDataType(), direction, topDataType));
                    }
                }
                structMember->setDataType(transformDataType(structMember->getDataType(), direction, topDataType));
                structMember->setContainList(containsList(structMember->getDataType()));
                structMember->setContainString(containsString(structMember->getDataType()));
            }
            break;
        }
        case DataType::kAliasType:
        {
            AliasType *aliasType = dynamic_cast<AliasType *>(dataType);
            assert(aliasType);
            aliasType->setElementType(transformDataType(aliasType->getElementType(), direction, topDataType));
            break;
        }
        default:
        {
            break;
        }
    }
    return dataType;
}

void CGenerator::transformDataTypes()
{
    for (auto it : m_globals->getSymbolsOfType(DataType::kAliasTypeSymbol))
    {
        AliasType *aliasType = dynamic_cast<AliasType *>(it);
        assert(aliasType);
        transformDataType(aliasType, kNone, aliasType);
    }

    for (auto it : m_globals->getSymbolsOfType(Symbol::kInterfaceSymbol))
    {
        Interface *iface = dynamic_cast<Interface *>(it);
        assert(iface);
        for (Function *fn : iface->getFunctions())
        {
            fn->setReturnType(transformDataType(fn->getReturnType(), kReturn, fn->getReturnType()));
            auto params = fn->getParameters().getMembers();
            for (auto mit : params)
            {
                setListLength(mit);
                mit->setDataType(transformDataType(mit->getDataType(), mit->getDirection(), mit->getDataType()));
            }
        }
    }
}

void CGenerator::setListLength(StructMember *structMember)
{
    Annotation *listLength = structMember->findAnnotation(LENGTH_ANNOTATION);
    if (listLength)
    {
        if (listLength->hasValue())
        {
            DataType *dataType = structMember->getDataType();
            if (dataType->isBinary())
            {
                BuiltinType *builtinType = dynamic_cast<BuiltinType *>(m_globals->getSymbol("uint8"));
                assert(builtinType);
                ListType *listType = new ListType(builtinType);
                structMember->setDataType(listType);
                dataType = listType;
                s_binaryTypes.push_back(listType);
            }
            if (dataType->getTrueDataType()->isList())
            {
                ListType *currentListType = dynamic_cast<ListType *>(dataType);
                assert(currentListType);
                currentListType->setLengthVariableName(listLength->getValueObject()->toString());
            }
        }
        else
        {
            throw semantic_error(format_string("Length annotation must have a value\n"));
        }
    }
}
void CGenerator::generate()
{
    m_templateData["generateInfraErrorChecks"] =
        (m_def->getErrorHandlingChecksType() == InterfaceDefinition::kAll ||
         m_def->getErrorHandlingChecksType() == InterfaceDefinition::kInfraErrors);
    m_templateData["generateAllocErrorChecks"] =
        (m_def->getErrorHandlingChecksType() == InterfaceDefinition::kAll ||
         m_def->getErrorHandlingChecksType() == InterfaceDefinition::kAllocErrors);

    data_list empty;
    m_templateData["enums"] = empty;
    m_templateData["aliases"] = empty;
    m_templateData["structs"] = empty;
    m_templateData["consts"] = empty;

    m_templateData["erpcgenVersion"] = ERPCGEN_VERSION;

    std::time_t now = std::time(nullptr);
    std::string nowString = std::ctime(&now);
    nowString.pop_back(); // Remove trailing newline.
    m_templateData["todaysDate"] = nowString;

    // Keil need extra pragma option when unions are used.
    m_templateData["usedUnionType"] = false;

    parseSubtemplates();

    if (m_def->hasProgramSymbol())
    {
        Log::info("program: ");
        Log::info("%s\n", m_def->getOutputFilename().c_str());
    }

    transformDataTypes();

    makeIncludesTemplateData();

    interfaceLists_t interfaceLists = makeInterfacesTemplateData();

    makeConstTemplateData();

    makeEnumsTemplateData();

    makeAliasesTemplateData();

    makeStructsTemplateData();

    // Generate output files. We have to special case not having any interfaces define.
    if (interfaceLists.empty())
    {
        // empty list of interfaces
        m_templateData["interfaces"] = empty;

        // Log template data.
        if (Log::getLogger()->getFilterLevel() >= Logger::kDebug2)
        {
            dump_data(m_templateData);
        }

        generateClientServerOutputFiles("");
    }
    else
    {
        for (auto interfaceList : interfaceLists)
        {
            m_templateData["interfaces"] = interfaceList.second;

            // Log template data.
            if (Log::getLogger()->getFilterLevel() >= Logger::kDebug2)
            {
                dump_data(m_templateData);
            }

            generateClientServerOutputFiles(interfaceList.first);
        }
    }
}

void CGenerator::makeIncludesTemplateData()
{
    data_list includeData;
    if (m_def->hasProgramSymbol())
    {
        for (auto include : m_def->programSymbol()->getAnnotations(INCLUDE_ANNOTATION))
        {
            includeData.push_back(make_data(include->getValueObject()->toString()));
            Log::info("#include %s\n", include->getValueObject()->toString().c_str());
        }
    }
    m_templateData["includes"] = includeData;
}

CGenerator::interfaceLists_t CGenerator::makeInterfacesTemplateData()
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
        ifaceInfo["serviceClassName"] = iface->getName() + "_service";
        ifaceInfo["mlComment"] = iface->getMlComment();
        ifaceInfo["ilComment"] = iface->getIlComment();

        Log::info("%d: (%d) %s\n", n, iface->getUniqueId(), iface->getName().c_str());
        // Log::info("%s\n", iface->printAnnotations().c_str());

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

void CGenerator::fillInterfaceListsWithMap(interfaceLists_t &interfaceLists,
                                           cpptempl::data_ptr interfaceMap,
                                           std::string mapName)
{
    auto interfaceList = interfaceLists.find(mapName);
    if (interfaceList == interfaceLists.end())
    {
        vector<cpptempl::data_ptr> interfaceVector;
        interfaceLists[mapName] = interfaceVector;
        interfaceList = interfaceLists.find(mapName);
    }
    interfaceList->second.push_back(interfaceMap);
}

data_list CGenerator::getFunctionsTemplateData(Interface *iface)
{
    data_list fns;
    int j = 0;
    for (auto fit : iface->getFunctions())
    {
        fns.push_back(make_data(getFunctionTemplateData(fit, j++)));
    }
    return fns;
}

void CGenerator::makeConstTemplateData()
{
    Log::info("Constant globals:\n");
    data_list consts;
    for (auto it : m_globals->getSymbolsOfType(Symbol::kConstSymbol))
    {
        ConstType *constVar = dynamic_cast<ConstType *>(it);
        assert(constVar);
        data_map constInfo;
        Annotation *externAnnotation = constVar->findAnnotation(EXTERNAL_ANNOTATION);
        if (!externAnnotation)
        {
            constInfo["typeAndName"] = getTypenameName(constVar->getDataType(), constVar->getName());
            // throw nullptr exception
            if (NULL == constVar->getValue())
            {
                Log::info("const pointing to null Value object\n");
            }
            if (kStringValue == constVar->getValue()->getType())
            {
                constInfo["value"] = "\"" + constVar->getValue()->toString() + "\"";
            }
            else
            {
                constInfo["value"] = constVar->getValue()->toString();
            }
            constInfo["mlComment"] = constVar->getMlComment();
            constInfo["ilComment"] = constVar->getIlComment();
            Log::info("Name=%s\tType=%s\tValue=%s\n", constVar->getName().c_str(),
                      constVar->getDataType()->getName().c_str(), constVar->getValue()->toString().c_str());
            consts.push_back(constInfo);
        }
    }
    m_templateData["consts"] = consts;
}

void CGenerator::makeEnumsTemplateData()
{
    Log::info("Enums:\n");
    data_list enums;
    int n = 0;
    for (auto it : m_globals->getSymbolsOfType(DataType::kEnumTypeSymbol))
    {
        EnumType *enumType = dynamic_cast<EnumType *>(it);
        assert(enumType);
        Annotation *externAnnotation = enumType->findAnnotation(EXTERNAL_ANNOTATION);
        if (!externAnnotation)
        {
            Log::info("%d: %s\n", n, enumType->getName().c_str());
            enums.push_back(getEnumTemplateData(enumType));
            ++n;
        }
    }
    m_templateData["enums"] = enums;
}

data_map CGenerator::getEnumTemplateData(EnumType *enumType)
{
    data_map enumInfo;
    enumInfo["name"] = enumType->getName();
    enumInfo["members"] = getEnumMembersTemplateData(enumType);
    enumInfo["mlComment"] = enumType->getMlComment();
    enumInfo["ilComment"] = enumType->getIlComment();
    return enumInfo;
}

data_list CGenerator::getEnumMembersTemplateData(EnumType *enumType)
{
    int j = 0;
    data_list enumMembersList;
    for (auto member : enumType->getMembers())
    {
        assert(member->hasValue());
        data_map enumMember;
        string memberDeclaration = member->getName();
        if (member->hasValue())
        {
            memberDeclaration = format_string("%s = %d", memberDeclaration.c_str(), member->getValue());
        }
        if (j + 1 < enumType->getMembers().size())
        {
            memberDeclaration += ",";
        }
        enumMember["memberDeclaration"] = memberDeclaration;
        Log::info("    %d: %s = %d\n", j, member->getName().c_str(), member->getValue());
        enumMember["mlComment"] = member->getMlComment();
        enumMember["ilComment"] = member->getIlComment();
        enumMembersList.push_back(enumMember);
        ++j;
    }
    return enumMembersList;
}

void CGenerator::makeAliasesTemplateData()
{
    Log::info("Type definition:\n");
    data_list aliases;
    int n = 0;
    for (auto it : m_globals->getSymbolsOfType(DataType::kAliasTypeSymbol))
    {
        AliasType *aliasType = dynamic_cast<AliasType *>(it);
        assert(aliasType);
        Annotation *externAnnotation = aliasType->findAnnotation(EXTERNAL_ANNOTATION);
        if (!externAnnotation)
        {
            Log::info("%d: ", n);
            data_map aliasInfo;
            DataType *elementDataType = aliasType->getElementType();

            if (elementDataType->getName() != "")
            {
                std::string realType = getTypenameName(elementDataType, aliasType->getName());
                Log::info("%s\n", realType.c_str());

                /* For case typedef struct */
                if (elementDataType->getName() == aliasType->getName())
                {
                    realType = "struct " + realType;
                }

                aliasInfo["typenameName"] = realType;
            }
            else
            {
                aliasInfo["typenameName"] = "";
                aliasInfo["unnamedName"] = aliasType->getName();
                switch (elementDataType->getDataType())
                {
                    case DataType::kStructType:
                    {
                        StructType *structType = dynamic_cast<StructType *>(elementDataType);
                        assert(structType);
                        aliasInfo["unnamed"] = getStructTemplateData(structType);
                        aliasInfo["unnamedType"] = "struct";
                        break;
                    }
                    case DataType::kEnumType:
                    {
                        EnumType *enumType = dynamic_cast<EnumType *>(elementDataType);
                        assert(enumType);
                        aliasInfo["unnamed"] = getEnumTemplateData(enumType);
                        aliasInfo["unnamedType"] = "enum";
                        break;
                    }
                    default:
                        throw internal_error("Only structs or enums are allowed as unnamed types.");
                }
            }
            aliasInfo["mlComment"] = aliasType->getMlComment();
            aliasInfo["ilComment"] = aliasType->getIlComment();
            aliases.push_back(aliasInfo);
            ++n;
        }
    }
    m_templateData["aliases"] = aliases;
}

AliasType *CGenerator::getAliasType(DataType *dataType)
{
    for (auto it : m_globals->getSymbolsOfType(DataType::kAliasTypeSymbol))
    {
        AliasType *aliasType = dynamic_cast<AliasType *>(it);
        assert(aliasType);
        DataType *elementType = aliasType->getElementType();
        if (elementType == dataType || aliasType == dataType)
        {
            return aliasType;
        }
    }
    return nullptr;
}

string CGenerator::getAliasName(DataType *dataType)
{
    AliasType *aliasType = getAliasType(dataType);
    return (aliasType != nullptr) ? aliasType->getName() : "";
}

void CGenerator::makeStructsTemplateData()
{
    Log::info("Structs:\n");
    data_list structs;
    data_list structsTo1Core;
    data_list structsTo2Core;
    data_list structsServerFree;
    m_templateData["nonExternalStruct"] = false;
    for (auto it : m_globals->getSymbolsOfType(Symbol::kStructTypeSymbol))
    {
        StructType *structType = dynamic_cast<StructType *>(it);
        assert(structType);
        Log::info("%s\n", structType->getDescription().c_str());
        data_map structInfo = getStructTemplateData(structType);
        if (structType->getName() == "")
        {
            structInfo["name"] = getAliasName(structType);
        }

        setToCore(structType, structsTo1Core, structsTo2Core, structInfo);
        structs.push_back(make_data(structInfo));
        if (structType->containStringMember() || structType->containListMember())
        {
            structsServerFree.push_back(structInfo);
        }
    }
    m_templateData["structs"] = structs;
    m_templateData["structsTo1Core"] = structsTo1Core;
    m_templateData["structsTo2Core"] = structsTo2Core;
    m_templateData["structsServerFree"] = structsServerFree;
}

data_map CGenerator::getStructTemplateData(StructType *structType)
{
    data_map structInfo;
    if (structType->findAnnotation(EXTERNAL_ANNOTATION))
    {
        structInfo["isExternal"] = true;
    }
    else
    {
        m_templateData["nonExternalStruct"] = true;
        structInfo["isExternal"] = false;
    }
    structInfo["name"] = make_data(structType->getName());
    structInfo["needEnumTmp"] = false;
    setStructMembersTemplateData(structType, structInfo);
    structInfo["mlComment"] = structType->getMlComment();
    structInfo["ilComment"] = structType->getIlComment();
    bool isNeedIfDealloc = false;
    if (structType->hasStructDirectionType(kInDirection) && structType->hasStructDirectionType(kInoutDirection))
    {
        isNeedIfDealloc = (structType->containListMember() || structType->containStringMember());
    }
    structInfo["isNeedIfDealloc"] = isNeedIfDealloc;
    structInfo["genStructWrapperF"] = !isBinaryStruct(structType);
    return structInfo;
}

void CGenerator::setStructMembersTemplateData(StructType *structType, cpptempl::data_map &structInfo)
{
    data_list members;
    data_list membersToFree;
    structInfo["hasNullableMember"] = false;
    uint32_t j = 0;
    for (auto member : structType->getMembers())
    {
        bool containsEnum;
        data_map member_info;
        DataType *dataType = member->getDataType();
        std::string memberName = member->getName();

        // Handle nullable annotation.
        if (dataType->isStruct() && (isListStruct(dynamic_cast<StructType *>(dataType)) ||
                                     isBinaryStruct(dynamic_cast<StructType *>(dataType))))
        {
            // Set a flag on the struct indicating there are nullable members, but only
            // set it to true
            if (member->findAnnotation(NULLABLE_ANNOTATION) != nullptr)
            {
                member_info["isNullable"] = true;
                structInfo["hasNullableMember"] = true;

                if (isListStruct(dynamic_cast<StructType *>(dataType)))
                {
                    member_info["structElements"] = ".elements";
                    member_info["structElementsCount"] = ".elementsCount";
                }
                else
                {
                    member_info["structElements"] = ".data";
                    member_info["structElementsCount"] = ".dataLength";
                }
            }
            else
            {
                member_info["isNullable"] = false;
            }
        }
        else
        {
            member_info["isNullable"] = false;
        }

        std::string name = "data->" + memberName;
        // Subtemplate setup for read/write struct calls
        member_info["coderCall"] = getEncodeDecodeCall(name, dataType, structType, true, member, containsEnum);
        // Info for delcaring struct in common header
        member_info["name"] = memberName;
        member_info["memberDeclaration"] = getTypenameName(dataType, memberName) + ";";
        if (isBinaryStruct(structType))
        {
            member_info["elementsCount"] = "uint32_t dataLength;";
        }
        else if (isListStruct(structType))
        {
            member_info["elementsCount"] = "uint32_t elementsCount;";
        }
        else
        {
            member_info["elementsCount"] = "";
        }
        member_info["mlComment"] = member->getMlComment();
        member_info["ilComment"] = member->getIlComment();
        if (containsEnum)
        {
            structInfo["needEnumTmp"] = true;
        }
        members.push_back(member_info);
        if (isNeedCallFree(dataType))
        {
            membersToFree.push_back(member_info);
        }
        j++;
    }
    structInfo["members"] = members;
    structInfo["membersToFree"] = membersToFree;
}

data_map CGenerator::getFunctionTemplateData(Function *fn, int fnIndex)
{
    data_map info;
    std::string proto = getFunctionPrototype(fn);
    std::string serverProto = getFunctionServerCall(fn);

    info["name"] = fn->getName();
    info["prototype"] = proto;
    info["serverPrototype"] = serverProto;
    info["id"] = fn->getUniqueId();
    info["isOneway"] = fn->isOneway();
    info["isReturnValue"] = !fn->isOneway();
    info["isSendValue"] = false;
    info["mlComment"] = fn->getMlComment();
    info["ilComment"] = fn->getIlComment();
    info["needEnumTmpServer"] = false;
    info["needEnumTmpClient"] = false;
    info["needNullVariableOnClient"] = false;
    info["needNullVariableOnServer"] = false;

    // Get return value info
    data_map returnInfo;
    returnInfo["type"] = getTypeInfo(fn->getReturnType(), true);
    DataType *dataType = fn->getReturnType();
    DataType *trueDataType = dataType->getTrueDataType();
    if (!trueDataType->isVoid())
    {
        std::string result = "result";
        bool containsEnum;
        returnInfo["firstAlloc"] = firstAllocOnReturnWhenIsNeed(result, dataType);
        setCallingFreeFunctions(dataType, returnInfo);
        std::string extraPointer = getExtraPointerInReturn(trueDataType);
        std::string resultVariable = extraPointer + returnSpaceWhenNotEmpty(extraPointer) + result;
        result = extraPointer + result;
        returnInfo["coderCall"] = getEncodeDecodeCall(result, dataType, nullptr, false, nullptr, containsEnum);
        resultVariable = getTypenameName(dataType, resultVariable);
        info["needEnumTmpClient"] = containsEnum;
        returnInfo["resultVariable"] = resultVariable;
        returnInfo["isNeedFreeingCall"] = isNeedCallFree(dataType);
        returnInfo["errorReturnValue"] = getErrorReturnValue(fn);
    }
    info["returnValue"] = returnInfo;
    // get function parameter info
    auto fnParams = fn->getParameters().getMembers();
    data_list params;
    data_list paramsToFree;
    data_list paramsTo1Core;
    data_list paramsTo2Core;
    for (StructMember *param : fnParams)
    {
        data_map paramInfo;
        bool containsEnum;
        DataType *paramType = param->getDataType();
        std::string name = param->getName();
        // Handle nullable annotation.
        if (paramType->isStruct())
        {
            paramInfo["isNullable"] = param->findAnnotation(NULLABLE_ANNOTATION) != nullptr;

            // Set flags to indicate whether a local isNull variable is needed on the
            // server and client sides.
            //
            // If needNullVariableX is true, we don't want to overwrite it to false
            // if a later parameter is not nullable. So, we will only try to set it
            // if it is not true. Once the variable's value is true, we know we need
            // a null variable at least once.
            if (param->getDirection() == kInDirection || param->getDirection() == kInoutDirection)
            {
                if ("false" == info["needNullVariableOnServer"]->getvalue())
                {
                    info["needNullVariableOnServer"] = paramInfo["isNullable"];
                }
            }
            if (param->getDirection() == kOutDirection || param->getDirection() == kInoutDirection)
            {
                if ("false" == info["needNullVariableOnClient"]->getvalue())
                {
                    info["needNullVariableOnClient"] = paramInfo["isNullable"];
                }
            }
        }
        else
        {
            paramInfo["isNullable"] = false;
        }
        paramInfo["mallocServer"] = firstAllocOnServerWhenIsNeed(name, param);
        paramInfo["mallocServerOut"] = firtAllocOutParamOnServerWhenIsNeed(name, param);
        paramInfo["mallocClient"] = firstAllocOnClientWhenIsNeed(name, param);
        setCallingFreeFunctions(param, paramInfo);
        name = getExtraDirectionPointer(param) + name;
        std::string variable = getTypenameName(paramType, name);
        paramInfo["name"] = name;
        paramInfo["type"] = getTypeInfo(paramType, false);
        Log::debug("calling EncodeDecode with %s as paramType\n", paramType->getName().c_str());
        paramInfo["coderCall"] = getEncodeDecodeCall(name, paramType, nullptr, false, param, containsEnum);
        paramInfo["variable"] = variable;
        setToCore(param, paramsTo1Core, paramsTo2Core, paramInfo);
        if (containsEnum && param->getDirection() != kInDirection)
        {
            info["needEnumTmpClient"] = true;
        }
        if (containsEnum && (param->getDirection() == kInDirection || param->getDirection() == kInoutDirection))
        {
            info["needEnumTmpServer"] = true;
        }
        params.push_back(paramInfo);

        // Generating top of freeing functions in generated output.
        bool l_generateServerFunctionParamFreeFunctions = generateServerFunctionParamFreeFunctions(param);
        bool isNeedFreeingCall = (l_generateServerFunctionParamFreeFunctions && isNeedCallFree(paramType));
        paramInfo["isNeedFreeingCall"] = isNeedFreeingCall;
        if (l_generateServerFunctionParamFreeFunctions &&
            (isNeedFreeingCall || paramInfo["firstFreeingCall1"]->getmap()["name"]->getvalue() != "" ||
             paramInfo["firstFreeingCall2"]->getmap()["name"]->getvalue() != ""))
        {
            paramsToFree.push_back(paramInfo);
        }
    }
    if (paramsTo1Core.size() > 0)
    {
        info["isReturnValue"] = true;
    }
    if (paramsTo2Core.size() > 0)
    {
        info["isSendValue"] = true;
    }
    info["parameters"] = make_data(params);
    info["paramsToFree"] = paramsToFree;
    info["parametersTo1Core"] = paramsTo1Core;
    info["parametersTo2Core"] = paramsTo2Core;

    Log::info("    %d: (%d) %s\n", fnIndex, fn->getUniqueId(), info["prototype"]->getvalue().c_str());

    return info;
}

void CGenerator::setToCore(Symbol *symbolType, data_list &to1Core, data_list &to2Core, data_map &dataMap)
{
    dataMap["allocateToServer"] = false;
    dataMap["allocateToClient"] = false;
    dataMap["allocateToClientAndServer"] = false;
    switch (getDirection(symbolType))
    {
        case 1: // in
        {
            dataMap["allocateToServer"] = true;
            to2Core.push_back(dataMap);
            break;
        }
        case 2: // out
        {
            dataMap["allocateToClient"] = true;
            to1Core.push_back(dataMap);
            break;
        }
        case 3: // inout
        {
            dataMap["allocateToClientAndServer"] = true;
            to2Core.push_back(dataMap);
            to1Core.push_back(dataMap);
            break;
        }
        default:
            throw std::runtime_error("internal error: unexpected parameter direction");
    }
}

data_map CGenerator::getTypeInfo(DataType *t, bool isFunction)
{
    data_map info;
    info["isNotVoid"] = make_data(t->getDataType() != DataType::kVoidType);
    return info;
}

std::string CGenerator::getErrorReturnValue(Function *fn)
{
    Annotation *ann = fn->findAnnotation(ERROR_RETURN_ANNOTATION);
    if (ann)
    {
        if (!ann->hasValue() || ann->getValueObject()->toString() == "")
        {
            throw semantic_error(format_string("Expected value for @%s annotation on line %d", ERROR_RETURN_ANNOTATION,
                                               ann->getLocation().m_firstLine));
        }
        else
        {
            std::string returnVal = ann->getValueObject()->toString();
            if (fn->getReturnType()->getTrueDataType()->isString())
            {
                return "(char *) " + returnVal;
            }
            return returnVal;
        }
    }
    else
    {
        DataType *dataType = fn->getReturnType()->getTrueDataType();
        if (dataType->isBuiltin() && !dataType->isString())
        {
            BuiltinType *builtinType = dynamic_cast<BuiltinType *>(dataType);
            switch (builtinType->getBuiltinType())
            {
                case BuiltinType::kBoolType:
                {
                    return "false";
                }
                case BuiltinType::kUInt8Type:
                {
                    return "0xFF";
                }
                case BuiltinType::kUInt16Type:
                {
                    return "0xFFFF";
                }
                case BuiltinType::kUInt32Type:
                {
                    return "0xFFFFFFFF";
                }
                case BuiltinType::kUInt64Type:
                {
                    return "0xFFFFFFFFFFFFFFFF";
                }
                default:
                {
                    return "-1";
                }
            }
        }
        else if (dataType->isEnum())
        {
            return "(" + fn->getReturnType()->getName() + ") -1";
        }
        else
        {
            return "NULL";
        }
    }
}

std::string CGenerator::getFunctionServerCall(Function *fn)
{
    std::string proto = "";
    if (!fn->getReturnType()->isVoid())
    {
        proto += "result = ";
    }
    proto += fn->getName();
    proto += "(";

    auto params = fn->getParameters().getMembers();

    if (params.size())
    {
        int n = 0;
        for (auto it : params)
        {
            bool isLast = (n == params.size() - 1);
            DataType *dataType = it->getDataType();
            DataType *trueDataType = it->getDataType()->getTrueDataType();
            if (dataType->isArray() && it->getDirection() == kInDirection)
            {
                std::string newDataType = getTypenameName(dataType, "*");
                proto += "(const " + newDataType + ") ";
            }
            else if ((trueDataType->isBuiltin() || trueDataType->isEnum()) &&
                     (it->getDirection() != kInDirection && !trueDataType->isString()))
            {
                proto += "&";
            }
            proto += it->getName();

            if (!isLast)
            {
                proto += ", ";
            }
            ++n;
        }
    }
    proto += ")";
    return proto;
}

std::string CGenerator::getFunctionPrototype(Function *fn)
{
    DataType *dataTypeReturn = fn->getReturnType();
    std::string proto = getExtraPointerInReturn(dataTypeReturn);
    if (proto == "*")
    {
        proto += " ";
    }
    proto += fn->getName();
    proto += "(";

    auto params = fn->getParameters().getMembers();
    if (params.size())
    {
        int n = 0;
        for (auto it : params)
        {
            bool isLast = (n == params.size() - 1);
            std::string paramSignature = it->getName();
            DataType *dataType = it->getDataType();
            DataType *trueDataType = dataType->getTrueDataType();
            if ((trueDataType->isBuiltin() || trueDataType->isEnum()) &&
                (it->getDirection() != kInDirection && !trueDataType->isString()))
            {
                paramSignature = "* " + paramSignature;
            }
            else
            {
                std::string directionPointer = getExtraDirectionPointer(it);
                paramSignature = directionPointer + returnSpaceWhenNotEmpty(directionPointer) + paramSignature;
            }
            paramSignature = getTypenameName(dataType, paramSignature);
            if ((dataType->isString() || trueDataType->isStruct() || trueDataType->isArray()) &&
                it->getDirection() == kInDirection)
            {
                proto += "const ";
            }

            DataType *trueContainerDataType = dataType->getTrueContainerDataType();
            if (trueContainerDataType->isStruct())
            {
                StructType *structType = dynamic_cast<StructType *>(trueContainerDataType);
                assert(structType);
                if (structType->hasStructDirectionType(kInoutDirection) &&
                    (structType->hasStructDirectionType(kOutDirection) ||
                     structType->hasStructDirectionType(kOutDirectionByref) ||
                     structType->hasStructDirectionType(kReturn)))
                {
                    throw syntax_error(
                        format_string("line %d: structs, lists, and binary cannot be used as both "
                                      "inout and out parameters in the same application",
                                      it->getLocation().m_firstLine));
                }
            }

            proto += paramSignature;
            if (!isLast)
            {
                proto += ", ";
            }
            ++n;
        }
    }
    else
    {
        proto += "void";
    }
    proto += ")";
    if (dataTypeReturn->isArray())
    {
        proto = "(" + proto + ")";
    }
    return getTypenameName(dataTypeReturn, proto); //! return type
}

std::string CGenerator::generateIncludeGuardName(const std::string &filename)
{
    std::string guard;

    // strip directory prefixes
    std::string fileNoPath = filename;
    size_t found = filename.find_last_of("/\\");
    if (found != std::string::npos)
    {
        std::string fileNoPath = filename.substr(found + 1);
    }
    // Create include guard macro name.
    guard = "_";
    guard += fileNoPath;
    guard += "_";
    size_t loc;
    while ((loc = guard.find_first_not_of(kIdentifierChars)) != guard.npos)
    {
        guard.replace(loc, 1, "_");
    }
    return guard;
}

std::string CGenerator::getTypenameName(DataType *t, const std::string &name)
{
    std::string returnName;
    switch (t->getDataType())
    {
        case DataType::kVoidType:
        {
            returnName = "void";
            returnName += returnSpaceWhenNotEmpty(name) + name;
            break;
        }
        case DataType::kBuiltinType:
        {
            assert(nullptr != dynamic_cast<const BuiltinType *>(t));
            returnName = getBuiltinTypename(dynamic_cast<const BuiltinType *>(t));
            if (!(t->isString() && name != "" && name[0] == '*'))
            {
                returnName += returnSpaceWhenNotEmpty(name);
            }
            returnName += name;
            break;
        }
        case DataType::kListType:
        {
            const ListType *a = dynamic_cast<const ListType *>(t);
            assert(a);
            returnName = "* " + name;
            returnName = getTypenameName(a->getElementType(), returnName);
            break;
        }
        case DataType::kArrayType:
        {
            // Array type requires the array element count to come after the variable/member name.
            returnName = name;
            ArrayType *a = dynamic_cast<ArrayType *>(t);
            assert(a);
            giveBracesToArrays(returnName);
            returnName = format_string("%s[%d]", returnName.c_str(), a->getElementCount());
            returnName = getTypenameName(a->getElementType(), returnName);
            break;
        }
        case DataType::kEnumType:
        {
            returnName = t->getName();
            returnName += returnSpaceWhenNotEmpty(name) + name;
            break;
        }
        case DataType::kStructType:
        {
            returnName = t->getName();
            returnName += returnSpaceWhenNotEmpty(name) + name;
            break;
        }
        case DataType::kAliasType:
        {
            returnName = t->getName();
            returnName += returnSpaceWhenNotEmpty(name) + name;
            break;
        }
        case DataType::kUnionType:
        {
            returnName += "union\n    {\n";
            assert(dynamic_cast<UnionType *>(t));
            for (auto unionCase : dynamic_cast<UnionType *>(t)->getUniqueCases())
            {
                if (!unionCase->caseMemberIsVoid())
                {
                    if (1 == unionCase->getMemberDeclarationNames().size())
                    {
                        std::string memberName = unionCase->getMemberDeclarationNames()[0];
                        StructMember *unionMember = unionCase->getUnionMemberDeclaration(memberName);
                        returnName += "        ";
                        returnName += getTypenameName(unionMember->getDataType(), unionMember->getName());
                        returnName += ";\n";
                    }
                    else
                    {
                        returnName += std::string("        ") + std::string("struct {\n");
                        for (auto memberName : unionCase->getMemberDeclarationNames())
                        {
                            StructMember *unionMember = unionCase->getUnionMemberDeclaration(memberName);
                            returnName += "            ";
                            returnName += getTypenameName(unionMember->getDataType(), unionMember->getName());
                            returnName += ";\n";
                        }
                        returnName += std::string("        ") + std::string("};\n");
                    }
                }
            }
            returnName += "    } " + name;
            break;
        }
        default:
            throw internal_error(format_string("In getTypenameName: unknown data type: %s value:%d",
                                               t->getName().c_str(), t->getDataType()));
    }
    return returnName;
}

std::string CGenerator::getBuiltinTypename(const BuiltinType *t)
{
    switch (t->getBuiltinType())
    {
        case BuiltinType::kBoolType:
            return "bool";
        case BuiltinType::kInt8Type:
            return "int8_t";
        case BuiltinType::kInt16Type:
            return "int16_t";
        case BuiltinType::kInt32Type:
            return "int32_t";
        case BuiltinType::kInt64Type:
            return "int64_t";
        case BuiltinType::kUInt8Type:
            return "uint8_t";
        case BuiltinType::kUInt16Type:
            return "uint16_t";
        case BuiltinType::kUInt32Type:
            return "uint32_t";
        case BuiltinType::kUInt64Type:
            return "uint64_t";
        case BuiltinType::kFloatType:
            return "float";
        case BuiltinType::kDoubleType:
            return "double";
        case BuiltinType::kStringType:
            return "char *";
        case BuiltinType::kBinaryType:
            return "uint8_t *";
        default:
            throw internal_error("unknown builtin type");
    }
}

void CGenerator::getEncodeDecodeBuiltin(BuiltinType *t,
                                        data_map &templateData,
                                        StructType *structType,
                                        StructMember *structMember)
{
    templateData["decode"] = m_templateData["decodeBuiltinType"];
    templateData["encode"] = m_templateData["encodeBuiltinType"];
    if (t->isString())
    {
        templateData["checkStringNull"] = false;
        // templateData["withoutAlloc"] = false;
        templateData["withoutAlloc"] = ((structMember && structMember->getDirection() == kInoutDirection) ||
                                        (structType && structType->hasStructDirectionType(kInoutDirection))) ?
                                           true :
                                           false;
        if (structType)
        {
            templateData["stringAllocSize"] = structMember->getName() + "_len";
            templateData["stringLocalName"] = structMember->getName();
        }
        else
        {
            if (!structMember)
            {
                templateData["stringAllocSize"] = "return_len";
                templateData["stringLocalName"] = "return";
            }
            else
            {
                templateData["checkStringNull"] = true;
                templateData["stringLocalName"] = structMember->getName();
                templateData["stringAllocSize"] = getMaxLength(structMember);
                if (structMember->getDirection() == kInoutDirection || structMember->getDirection() == kOutDirection)
                {
                    templateData["withoutAlloc"] = true;
                }

                if (templateData["stringAllocSize"]->getvalue() == "")
                {
                    templateData["stringAllocSize"] = templateData["stringLocalName"]->getvalue() + "_len";
                }
            }
        }
        templateData["freeingCall"] = m_templateData["freeData"];
        // needDealloc(templateData, t, structType, nullptr);
        templateData["builtinType"] = "kStringType";
    }
    else
    {
        templateData["builtinType"] = "kNumberType";
    }
}

// TODO: Return error if maxSize < receivied size
std::string CGenerator::getMaxLength(StructMember *structMember)
{
    Annotation *an = structMember->findAnnotation(MAX_LENGTH_ANNOTATION);
    return (an) ? an->getValueObject()->toString() : "";
}

data_map CGenerator::getEncodeDecodeCall(const std::string &name,
                                         DataType *t,
                                         StructType *structType,
                                         bool inDataContainer,
                                         StructMember *structMember,
                                         bool &containsEnum)
{
    // prepare data for template
    data_map templateData;
    string_vector params = string_vector(1, "");
    templateData["freeingCall"] = make_template("", &params);
    templateData["isElementArrayType"] = false;

    int32_t pos = name.rfind("*");
    if ((pos == 0 || pos == 1) && t->getTrueDataType()->isStruct() && inDataContainer == false)
    {
        templateData["name"] = name.substr(1, name.length());
    }
    else
    {
        templateData["name"] = name;
    }

    if ((t->isBuiltin() || t->isEnum()) && !t->isString() && !t->isBinary())
    {
        templateData["pointerScalarTypes"] = false;
        if (!inDataContainer && structMember && structMember->getDirection() != kInDirection)
        {
            DataType *trueDataType = t->getTrueDataType();
            if ((trueDataType->isBuiltin() || trueDataType->isEnum()) && !trueDataType->isString())
            {
                templateData["pointerScalarTypes"] = true;
            }
        }
    }

    containsEnum = t->isEnum();

    switch (t->getDataType())
    {
        case DataType::kBuiltinType:
        {
            getEncodeDecodeBuiltin((BuiltinType *)t, templateData, structType, structMember);
            break;
        }
        case DataType::kEnumType:
        {
            containsEnum = true;
            templateData["decode"] = m_templateData["decodeEnumType"];
            templateData["encode"] = m_templateData["encodeEnumType"];
            std::string typeName = t->getName();
            if (typeName != "")
            {
                templateData["enumName"] = typeName;
            }
            else
            {
                templateData["enumName"] = getAliasName(t);
            }
            break;
        }
        case DataType::kListType:
        {
            ListType *listType = dynamic_cast<ListType *>(t);
            assert(listType);

            bool isInOut = ((structMember && structMember->getDirection() == kInoutDirection) ||
                            (structType && structType->hasStructDirectionType(kInoutDirection)));
            bool isTopDataTYpe = (!structType && structMember && structMember->getDataType()->getTrueDataType() == t);
            templateData["useMallocOnClientSide"] = (!isInOut && !isTopDataTYpe); // Because cpptempl don't know do
                                                                                  // correct complicated conditions like
                                                                                  // if(a || (b && c))

            templateData["mallocSizeType"] = getTypenameName(listType->getElementType(), "");
            templateData["mallocType"] = getTypenameName(listType->getElementType(), "*");
            templateData["needFreeingCall"] =
                (generateServerFunctionParamFreeFunctions(structMember) && isNeedCallFree(listType->getElementType()));
            templateData["freeingCall"] = (generateServerFunctionParamFreeFunctions(structMember)) ?
                                              m_templateData["freeList"] :
                                              make_template("", &params);
            templateData["decode"] = m_templateData["decodeListType"];
            templateData["encode"] = m_templateData["encodeListType"];
            std::string nextName;

            // Size variable should have same prefix as variable name.
            auto sizePrefix = name.rfind('.');
            if (sizePrefix == std::string::npos)
            {
                sizePrefix = name.rfind('>');
            }
            std::string size = "";
            if (sizePrefix != std::string::npos)
            {
                size = name.substr(0, sizePrefix + 1);
            }

            if (listType->hasLengthVariable())
            {
                templateData["hasLengthVariable"] = true;
                static uint32_t listCounter;
                ++listCounter;
                nextName = format_string("%s[listCount%d]", name.c_str(), listCounter);
                // needDealloc(templateData, t, nullptr, structMember);
                size += listType->getLengthVariableName();
                templateData["sizeTemp"] = std::string("lengthTemp_") + std::to_string(listCounter);
                std::string maxSize = getMaxLength(structMember);
                if (maxSize != "")
                {
                    templateData["maxSize"] = maxSize;
                }
                else
                {
                    templateData["maxSize"] = templateData["sizeTemp"]->getvalue();
                }
                templateData["forLoopCount"] = std::string("listCount") + std::to_string(listCounter);
                templateData["isElementArrayType"] = listType->getElementType()->getTrueDataType()->isArray();
            }
            else
            {
                templateData["hasLengthVariable"] = false;
                if (sizePrefix != std::string::npos)
                {
                    std::string usedName = name.substr(sizePrefix + 1, name.size() - sizePrefix - 1);
                    if (usedName == "data")
                    {
                        size += "dataLength";
                    }
                    else
                    {
                        size += "elementsCount";
                    }
                    templateData["sizeTemp"] = size;
                    templateData["maxSize"] = size;
                }
                else
                {
                    throw internal_error("Unexpected error with List data type.");
                }
                // needDealloc(templateData, t, structType, nullptr);
                nextName = name + "[listCount]";
                templateData["forLoopCount"] = "listCount";
            }
            templateData["size"] = size;
            templateData["useBinaryCoder"] = false;
            for (ListType *binaryType : s_binaryTypes)
            {
                if (binaryType == listType)
                {
                    templateData["useBinaryCoder"] = true;
                    if (!listType->hasLengthVariable())
                    {
                        templateData["sizeTemp"] = "data_local";
                    }
                    else
                    {
                        templateData["maxSize"] = templateData["size"]->getvalue();
                    }
                    break;
                }
            }
            templateData["protoNext"] =
                getEncodeDecodeCall(nextName, listType->getElementType(), structType, true, structMember, containsEnum);
            break;
        }
        case DataType::kArrayType:
        {
            static uint32_t arrayCounter;
            ArrayType *arrayType = dynamic_cast<ArrayType *>(t);
            assert(arrayType);
            std::string arrayName = name;
            ++arrayCounter;
            templateData["decode"] = m_templateData["decodeArrayType"];
            templateData["encode"] = m_templateData["encodeArrayType"];
            giveBracesToArrays(arrayName);
            templateData["protoNext"] =
                getEncodeDecodeCall(format_string("%s[arrayCount%d]", arrayName.c_str(), arrayCounter),
                                    arrayType->getElementType(), structType, true, structMember, containsEnum);
            templateData["forLoopCount"] = format_string("arrayCount%d", arrayCounter);
            templateData["size"] = format_string("%d", arrayType->getElementCount());
            templateData["sizeTemp"] = format_string("%d", arrayType->getElementCount());
            templateData["isElementArrayType"] = arrayType->getElementType()->getTrueDataType()->isArray();
            --arrayCounter;
            templateData["freeingCall"] =
                (generateServerFunctionParamFreeFunctions(structMember) && isNeedCallFree(t)) ?
                    m_templateData["freeArray"] :
                    make_template("", &params);
            break;
        }
        case DataType::kStructType:
        {
            templateData["inDataContainer"] = inDataContainer;
            // needDealloc(templateData, t, structType, structMember);
            std::string typeName = t->getName();
            if (typeName != "")
            {
                templateData["typeName"] = typeName;
            }
            else
            {
                templateData["typeName"] = getAliasName(t);
            }
            templateData["decode"] = m_templateData["decodeStructType"];
            templateData["encode"] = m_templateData["encodeStructType"];
            templateData["freeingCall"] =
                (generateServerFunctionParamFreeFunctions(structMember) && isNeedCallFree(t)) ?
                    m_templateData["freeStruct"] :
                    make_template("", &params);
            break;
        }
        case DataType::kAliasType:
        {
            AliasType *aliasType = dynamic_cast<AliasType *>(t);
            assert(aliasType);
            return getEncodeDecodeCall(name, aliasType->getElementType(), structType, inDataContainer, structMember,
                                       containsEnum);
        }
        case DataType::kUnionType:
        {
            UnionType *unionType = dynamic_cast<UnionType *>(t);
            assert(unionType);
            templateData["dataLiteral"] = "data";
            templateData["discriminatorName"] = unionType->getDiscriminatorName();
            data_list unionCases;
            data_list unionCasesToFree;
            templateData["freeingCall"] = make_template("", &params);
            for (auto unionCase : unionType->getCases())
            {
                data_map caseData;
                caseData["name"] = unionCase->getCaseName();
                caseData["value"] = unionCase->getCaseValue();
                data_list caseMembers;
                data_map memberData;
                data_map caseMembersFree;
                if (unionCase->caseMemberIsVoid())
                {
                    // Create phony enocde/decode values for void function,
                    // since we don't want to generate anything.
                    data_map coderCalls;
                    coderCalls["encode"] = make_template("", &params);
                    coderCalls["decode"] = make_template("", &params);
                    memberData["coderCall"] = coderCalls;

                    caseMembers.push_back(memberData);
                }
                else
                {
                    // For each case member declaration, get its encode and decode data
                    caseData["needCaseFreeingCall"] = false;
                    for (auto caseMemberName : unionCase->getMemberDeclarationNames())
                    {
                        StructMember *memberDeclaration = unionCase->getUnionMemberDeclaration(caseMemberName);
                        std::string unionCaseName = name + "." + memberDeclaration->getName();
                        memberData["coderCall"] = getEncodeDecodeCall(unionCaseName, memberDeclaration->getDataType(),
                                                                      structType, true, structMember, containsEnum);
                        if (generateServerFunctionParamFreeFunctions(structMember) &&
                            isNeedCallFree(memberDeclaration->getDataType()))
                        {
                            templateData["freeingCall"] = m_templateData["freeUnion"];
                            caseData["needCaseFreeingCall"] = true;
                            memberData["isNeedFreeingCall"] = true;
                        }
                        else
                        {
                            memberData["isNeedFreeingCall"] = false;
                        }
                        caseMembers.push_back(memberData);
                    }
                }
                caseData["members"] = caseMembers;
                unionCases.push_back(caseData);
            }
            templateData["cases"] = unionCases;
            templateData["encode"] = m_templateData["encodeUnionType"];
            templateData["decode"] = m_templateData["decodeUnionType"];
            break;
        }
        default:
        {
            throw internal_error("unknown member type");
        }
    }
    return templateData;
}

void CGenerator::giveBracesToArrays(std::string &name)
{
    if (name.size() && name.substr(0, 1) == "*")
    {
        name = "(" + name + ")";
    }
}

std::string CGenerator::getExtraPointerInReturn(DataType *dataType)
{
    DataType *trueDataType = dataType->getTrueDataType();
    if (trueDataType->isStruct() || trueDataType->isArray())
    {
        return "*";
    }

    return "";
}

std::string CGenerator::getExtraDirectionPointer(StructMember *structMember)
{
    DataType *dataType = structMember->getDataType();
    DataType *trueDataType = dataType->getTrueDataType();
    _param_direction structMemberDir = structMember->getDirection();
    std::string result;
    if (structMemberDir == kOutDirection) // between out and inout can be diferences in future. Maybe not.
    {
        if (!trueDataType->isBuiltin() && !trueDataType->isEnum() && !trueDataType->isList())
        {
            result = "*";
        }
    }
    else if (structMemberDir == kInoutDirection)
    {
        if (!trueDataType->isBuiltin() && !trueDataType->isEnum() && !trueDataType->isList())
        {
            result = "*";
        }
    }
    else if (structMemberDir == kOutDirectionByref)
    {
        if (trueDataType->isString())
        {
            result = "*";
        }
        else if (!trueDataType->isBuiltin() && !trueDataType->isEnum())
        {
            result = "**";
        }
    }
    else //! Last else is for InDirection, which has same principle as return;
    {
        result = getExtraPointerInReturn(dataType);
    }

    return result;
}

std::string CGenerator::firstAllocOnReturnWhenIsNeed(std::string name, DataType *dataType)
{
    DataType *trueDataType = dataType->getTrueDataType();
    if (trueDataType->isArray() || trueDataType->isStruct())
    {
        return allocateCall(false, name, dataType);
    }

    return "";
}

std::string CGenerator::firstAllocOnServerWhenIsNeed(std::string name, StructMember *structMember)
{
    DataType *dataType = structMember->getDataType();
    DataType *trueDataType = dataType->getTrueDataType();
    _param_direction structMemberDir = structMember->getDirection();
    if (structMemberDir == kInoutDirection)
    {
        if (!trueDataType->isBuiltin() && !trueDataType->isEnum() && !trueDataType->isList())
        {
            return allocateCall(false, name, structMember);
        }
    }
    else if (structMemberDir == kInDirection)
    {
        if (trueDataType->isArray() || trueDataType->isStruct())
        {
            return allocateCall(false, name, structMember);
        }
    }
    else if (structMemberDir == kOutDirectionByref)
    {
        if (!trueDataType->isBuiltin() && !trueDataType->isEnum())
        {
            std::string result = allocateCall(true, name, structMember);
            name = "*" + name;
            return result + "\n" + allocateCall(false, name, structMember);
        }
        else
        {
            if (trueDataType->isString())
            {
                std::string result = allocateCall(true, name, structMember);
                return result += "\n" + format_string("*%s = NULL;", name.c_str());
            }
        }
    }
    return "";
}

std::string CGenerator::firtAllocOutParamOnServerWhenIsNeed(std::string name, StructMember *structMember)
{
    if (structMember->getDirection() == kOutDirection)
    {
        DataType *trueDataType = structMember->getDataType()->getTrueDataType();
        if (!trueDataType->isBuiltin() && !trueDataType->isEnum())
        {
            return allocateCall(false, name, structMember);
        }
        else if (trueDataType->isString())
        {
            if (!structMember->findAnnotation(MAX_LENGTH_ANNOTATION))
            {
                Symbol *symbol = structMember;
                assert(symbol);
                throw semantic_error(
                    format_string("For out string variable '%s' on line '%d' max_length annotation has to be set.",
                                  symbol->getName().c_str(), symbol->getLocation().m_firstLine));
            }
            return allocateCall(false, name, structMember) + format_string("\n%s[0]=\'\\0\';", name.c_str());
        }
    }
    return "";
}

std::string CGenerator::firstAllocOnClientWhenIsNeed(std::string name, StructMember *structMember)
{
    DataType *dataType = structMember->getDataType();
    DataType *trueDataType = dataType->getTrueDataType();
    if (structMember->getDirection() == kOutDirectionByref)
    {
        if (!trueDataType->isBuiltin() && !trueDataType->isEnum() && !trueDataType->isList())
        {
            name = "* " + name;
            return allocateCall(false, name, structMember);
        }
    }
    return "";
}

bool CGenerator::isNeedCallFree(DataType *dataType)
{
    DataType *trueDataType = dataType->getTrueDataType();
    switch (trueDataType->getDataType())
    {
        case DataType::kStructType:
        {
            StructType *structType = dynamic_cast<StructType *>(trueDataType);
            assert(structType);
            return structType->containListMember() || structType->containStringMember();
        }
        case DataType::kArrayType:
        {
            ArrayType *arrayType = dynamic_cast<ArrayType *>(trueDataType);
            assert(arrayType);
            return isNeedCallFree(arrayType->getElementType());
        }
        case DataType::kBuiltinType:
        {
            BuiltinType *builtinType = dynamic_cast<BuiltinType *>(trueDataType);
            assert(builtinType);
            return builtinType->isString() || builtinType->isBinary();
        }
        case DataType::kListType:
        {
            return true;
        }
        case DataType::kUnionType:
        {
            UnionType *unionType = dynamic_cast<UnionType *>(trueDataType);
            assert(unionType);
            for (auto unionCase : unionType->getCases())
            {
                if (!unionCase->caseMemberIsVoid())
                {
                    for (auto caseMemberName : unionCase->getMemberDeclarationNames())
                    {
                        StructMember *memberDeclaration = unionCase->getUnionMemberDeclaration(caseMemberName);
                        if (isNeedCallFree(memberDeclaration->getDataType()))
                        {
                            return true;
                        }
                    }
                }
            }
            return false;
        }
        default:
            return false;
    }
}

void CGenerator::setCallingFreeFunctions(Symbol *symbol, cpptempl::data_map &info)
{
    StructMember *structMember = dynamic_cast<StructMember *>(symbol);
    data_map firstFreeingCall1;
    firstFreeingCall1["firstFreeingCall"] = "";
    firstFreeingCall1["name"] = "";
    data_map firstFreeingCall2;
    firstFreeingCall2["firstFreeingCall"] = "";
    firstFreeingCall2["name"] = "";
    // When true then function parameter, else return type
    if (structMember)
    {
        DataType *trueDataType = structMember->getDataType()->getTrueDataType();
        switch (trueDataType->getDataType())
        {
            case DataType::kBuiltinType:
            case DataType::kArrayType:
            case DataType::kStructType:
            {
                BuiltinType *builtinType = dynamic_cast<BuiltinType *>(trueDataType);
                if (!builtinType || builtinType->isString())
                {
                    _param_direction structMemberDir = structMember->getDirection();
                    std::string name = structMember->getName();
                    if (!((trueDataType->isString()) && structMemberDir != kOutDirectionByref))
                    {
                        firstFreeingCall1["firstFreeingCall"] = m_templateData["freeData"];
                        firstFreeingCall1["name"] = name;
                    }
                    if (!trueDataType->isString() && structMemberDir == kOutDirectionByref)
                    {
                        name = "*" + name;
                        firstFreeingCall2["firstFreeingCall"] = m_templateData["freeData"];
                        firstFreeingCall2["name"] = name;
                    }
                }
            }
            default:
                break;
        }
    }
    else
    {
        DataType *dataType = dynamic_cast<DataType *>(symbol);
        assert(dataType);
        if (dataType->getTrueDataType()->isArray() || dataType->getTrueDataType()->isStruct())
        {
            firstFreeingCall1["firstFreeingCall"] = m_templateData["freeData"];
            firstFreeingCall1["name"] = "result";
        }
    }
    info["firstFreeingCall1"] = firstFreeingCall1;
    info["firstFreeingCall2"] = firstFreeingCall2;
}

std::string CGenerator::allocateCall(bool pointer, std::string &name, Symbol *symbol)
{
    StructMember *structMember = dynamic_cast<StructMember *>(symbol);
    DataType *dataType;
    if (structMember)
    {
        dataType = structMember->getDataType();
    }
    else
    {
        dataType = dynamic_cast<DataType *>(symbol);
    }
    DataType *trueDataType = dataType->getTrueDataType();
    std::string pointers;
    std::string size;
    if (pointer == true)
    {
        pointers = "*";
    }
    if ((trueDataType->isList() || trueDataType->isString()) && structMember)
    {
        size = getMaxLength(structMember);
        if (size != "")
        {
            if (dataType->isString() && pointers != "*")
            {
                size = "(" + size + " + 1) * ";
            }
            else
            {
                size += " * ";
            }
        }
    }
    if (dataType->isList())
    {
        ListType *listType = dynamic_cast<ListType *>(dataType);
        assert(listType);
        dataType = listType->getElementType();
        if (size == "")
        {
            size = listType->getLengthVariableName() + " * ";
        }
    }
    std::string typeValue;
    std::string typePointerValue;
    if (!dataType->isString())
    {
        typeValue = getTypenameName(dataType, pointers);
        typePointerValue = getTypenameName(dataType, pointers + "*");
    }
    else
    {
        typeValue = "char" + pointers;
        typePointerValue = "char *" + pointers;
    }
    std::string returnVal = format_string("%s = (%s) erpc_malloc(%ssizeof(%s));", name.c_str(),
                                          typePointerValue.c_str(), size.c_str(), typeValue.c_str());
    if (m_templateData["generateInfraErrorChecks"]->getvalue() == "true")
    {
        returnVal += format_string("\nif (%s == NULL)\n{\n    err = kErpcStatus_MemoryError;\n}", name.c_str());
    }
    return returnVal;
}

std::string CGenerator::returnSpaceWhenNotEmpty(const std::string &param)
{
    return (param == "") ? "" : " ";
}

uint8_t CGenerator::getDirection(Symbol *symbolType)
{
    if (symbolType->getSymbolType() == Symbol::kStructTypeSymbol)
    {
        StructType *structType = dynamic_cast<StructType *>(symbolType);
        assert(structType);
        bool in = structType->hasStructDirectionType(kInDirection);
        bool out = structType->hasStructDirectionType(kOutDirection);
        bool outByref = structType->hasStructDirectionType(kOutDirectionByref);
        bool inOut = structType->hasStructDirectionType(kInoutDirection);
        bool ret = structType->hasStructDirectionType(kReturn);

        if (inOut || (in && (outByref || ret || out)))
        {
            return 3;
        }
        else if (outByref || ret || out)
        {
            return 2;
        }
    }
    else if (symbolType->getSymbolType() == Symbol::kStructMemberSymbol)
    {
        StructMember *structMember = dynamic_cast<StructMember *>(symbolType);
        assert(structMember);
        switch (structMember->getDirection())
        {
            case kOutDirection:
            case kOutDirectionByref:
            {
                return 2;
            }
            case kInoutDirection:
            {
                return 3;
            }
            case kInDirection:
            case kReturn:
            {
                break;
            }
            default:
            {
                throw internal_error("Unsupported direction type of structure member.");
            }
        }
    }
    else
    {
        throw internal_error(
            format_string("Symbol: %s is not structure or function parameter.", symbolType->getDescription().c_str()));
    }
    return 1;
}

bool CGenerator::containsString(DataType *dataType)
{
    if (dataType->getTrueDataType()->isList())
    {
        ListType *l = dynamic_cast<ListType *>(dataType->getTrueDataType());
        assert(l);
        return containsString(l->getElementType());
    }
    DataType *trueDataType = dataType->getTrueContainerDataType();
    switch (trueDataType->getDataType())
    {
        case DataType::kStructType:
        {
            StructType *s = dynamic_cast<StructType *>(trueDataType);
            assert(s);
            return s->containStringMember();
        }
        case DataType::kUnionType:
        {
            UnionType *u = dynamic_cast<UnionType *>(trueDataType);
            assert(u);
            for (UnionCase *unionCase : u->getUniqueCases())
            {
                if (!unionCase->caseMemberIsVoid())
                {
                    for (std::string unionCaseName : unionCase->getMemberDeclarationNames())
                    {
                        StructMember *unionCaseMember = unionCase->getUnionMemberDeclaration(unionCaseName);
                        if (containsString(unionCaseMember->getDataType()))
                        {
                            return true;
                        }
                    }
                }
            }
            return false;
        }
        default:
        {
            if (trueDataType->isString())
            {
                return true;
            }
            return false;
        }
    }
}

bool CGenerator::containsList(DataType *dataType)
{
    if (dataType->getTrueDataType()->isList())
    {
        return true;
    }
    DataType *trueDataType = dataType->getTrueContainerDataType();
    switch (trueDataType->getDataType())
    {
        case DataType::kStructType:
        {
            StructType *s = dynamic_cast<StructType *>(trueDataType);
            assert(s);
            return s->containListMember();
        }
        case DataType::kUnionType:
        {
            UnionType *u = dynamic_cast<UnionType *>(trueDataType);
            assert(u);
            for (UnionCase *unionCase : u->getUniqueCases())
            {
                if (!unionCase->caseMemberIsVoid())
                {
                    for (std::string unionCaseName : unionCase->getMemberDeclarationNames())
                    {
                        StructMember *unionCaseMember = unionCase->getUnionMemberDeclaration(unionCaseName);
                        if (containsList(unionCaseMember->getDataType()))
                        {
                            return true;
                        }
                    }
                }
            }
            return false;
        }
        default:
        {
            return false;
        }
    }
}

bool CGenerator::isListStruct(StructType *structType)
{
    std::string structName = structType->getName();
    if (structName != "binary_t")
    {
        uint32_t nameCount = 0;
        std::string structuresNames = format_string("list_%d_t", nameCount);
        Symbol *symbol = m_globals->getSymbol(structuresNames);
        while (symbol)
        {
            if (structuresNames == structName)
            {
                break;
            }
            ++nameCount;
            structuresNames = format_string("list_%d_t", nameCount);
            symbol = m_globals->getSymbol(structuresNames);
        }
        if (!symbol)
        {
            return false;
        }
    }

    StructType::member_vector_t &structMembers = structType->getMembers();
    if (structMembers.size() == 1 &&
        (structMembers[0]->getName() == "elements" || structMembers[0]->getName() == "data"))
    {
        DataType *dataType = structMembers[0]->getDataType();
        if (dataType->isList())
        {
            ListType *listType = dynamic_cast<ListType *>(dataType);
            return !listType->hasLengthVariable();
        }
    }
    return false;
}

bool CGenerator::isBinaryStruct(StructType *structType)
{
    return (structType->getName() == "binary_t" && structType->getMembers().size() == 1 &&
            structType->getMembers()[0]->getDataType()->isList());
}

bool CGenerator::generateServerFunctionParamFreeFunctions(StructMember *structMember)
{
    return (structMember == nullptr || structMember->findAnnotation(RETAIN_ANNOTATION) == nullptr);
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
