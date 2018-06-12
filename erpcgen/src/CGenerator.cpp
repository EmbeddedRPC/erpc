/*
 * The Clear BSD License
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 *  that the following conditions are met:
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
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
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
#include "Logging.h"
#include "ParseErrors.h"
#include "annotations.h"
#include "format_string.h"
#include <algorithm>
#include <set>
#include <sstream>

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
extern const char *const kCDefines;
extern const char *const kCCrc;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////
CGenerator::CGenerator(InterfaceDefinition *def, uint16_t idlCrc16)
: Generator(def, idlCrc16)
{
    /* Set copyright rules. */
    if (m_def->hasProgramSymbol())
    {
        Program *program = m_def->getProgramSymbol();
        std::string copyright = program->getMlComment();
        if (copyright.size() >= 3 && (copyright[2] == '*' || copyright[2] == '!'))
        {
            copyright = copyright.substr(0, 2) + copyright.substr(3, copyright.size() - 3);
            program->setMlComment(copyright);
        }
        setTemplateComments(program, m_templateData);
    }
    else
    {
        m_templateData["mlComment"] = "";
        m_templateData["ilComment"] = "";
    }
}

void CGenerator::generateOutputFiles(const std::string &fileName)
{
    generateClientSourceFile(fileName);

    generateServerHeaderFile(fileName);
    generateServerSourceFile(fileName);

    generateCommonHeaderFiles(fileName);
}

void CGenerator::generateTypesHeaderFile()
{
    std::string typesHeaderFileName = m_templateData["commonTypesFile"]->getvalue();

    m_templateData["commonGuardMacro"] = generateIncludeGuardName(typesHeaderFileName);
    m_templateData["genCommonTypesFile"] = true;
    m_templateData["commonTypesFile"] = "";

    generateOutputFile(typesHeaderFileName, "c_common_header", m_templateData, kCCommonHeader);
}

void CGenerator::generateCommonHeaderFiles(const std::string &fileName)
{
    m_templateData["commonGuardMacro"] = generateIncludeGuardName(fileName + ".h");
    m_templateData["genCommonTypesFile"] = false;

    generateOutputFile(fileName + ".h", "c_common_header", m_templateData, kCCommonHeader);
}

void CGenerator::generateClientSourceFile(std::string fileName)
{
    m_templateData["source"] = "client";
    fileName += "_client.cpp";
    m_templateData["clientSourceName"] = fileName;

    //TODO: temporary workaround for tests
    m_templateData["unitTest"] = (fileName.compare("test_unit_test_common_client.cpp") == 0 ? false : true);

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

    //TODO: temporary workaround for tests
    m_templateData["unitTest"] = (fileName.compare("test_unit_test_common_server.cpp") == 0 ? false : true);

    generateOutputFile(fileName, "c_server_source", m_templateData, kCServerSource);
}

void CGenerator::generateCrcFile()
{
    std::string filename = "erpc_crc16.h";
    m_templateData["crcGuardMacro"] = generateIncludeGuardName(filename);
    generateOutputFile(filename, "c_crc", m_templateData, kCCrc);
}

void CGenerator::parseSubtemplates()
{
    std::string templateName = "c_coders";
    try
    {
        parse(kCCoders, m_templateData);
        templateName = "c_common_functions";
        parse(kCCommonFunctions, m_templateData);
        templateName = "c_defines";
        parse(kCDefines, m_templateData);
    }
    catch (TemplateException &e)
    {
        throw TemplateException(format_string("Template %s: %s", templateName.c_str(), e.what()));
    }
}

DataType *CGenerator::findChildDataType(std::set<DataType *> &dataTypes, DataType *dataType)
{
    // Detecting loops from forward declarations.
    // Insert data type into set
    if (!(dataType->isBinary() || dataType->isList()))
    {
        if (!dataTypes.insert(dataType).second)
        {
            return dataType;
        }
    }

    switch (dataType->getDataType())
    {
        case DataType::kAliasType:
        {
            AliasType *aliasType = dynamic_cast<AliasType *>(dataType);
            assert(aliasType);
            aliasType->setElementType(findChildDataType(dataTypes, aliasType->getElementType()));
            break;
        }
        case DataType::kArrayType:
        {
            ArrayType *arrayType = dynamic_cast<ArrayType *>(dataType);
            assert(arrayType);
            arrayType->setElementType(findChildDataType(dataTypes, arrayType->getElementType()));
            break;
        }
        case DataType::kBuiltinType:
        {
            if (dataType->isBinary())
            {
                // check if binary data type was replaced with structure wrapper
                dataType = dynamic_cast<DataType *>(m_globals->getSymbol("binary_t"));
                if (!dataType)
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

                    m_globals->replaceSymbol(replacedBuiltinType, newStruct);
                    m_listBinaryTypes.insert(m_listBinaryTypes.begin(), listType);

                    dataType = dynamic_cast<DataType *>(m_globals->getSymbol("binary_t"));
                    assert(dataType);
                }
            }
            dataTypes.insert(dataType);
            break;
        }
        case DataType::kFunctionType:
        {
            FunctionType *funcType = dynamic_cast<FunctionType *>(dataType);
            assert(funcType);

            // Only for detecting loop from forward declaration.
            std::set<DataType *> localDataTypes;
            localDataTypes.insert(dataTypes.begin(), dataTypes.end());

            // handle return value
            StructMember *returnType = funcType->getReturnStructMemberType();
            DataType *transformedDataType = findChildDataType(localDataTypes, funcType->getReturnType());
            returnType->setDataType(transformedDataType);

            // handle function parameters
            auto params = funcType->getParameters().getMembers();
            for (auto mit : params)
            {
                setBinaryList(mit);

                mit->setDataType(findChildDataType(localDataTypes, mit->getDataType()));
            }
            break;
        }
        case DataType::kListType:
        {
            // The only child node of a list node is the element type.
            ListType *listType = dynamic_cast<ListType *>(dataType);
            assert(listType);
            DataType *elementType = findChildDataType(dataTypes, listType->getElementType());
            listType->setElementType(elementType);

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
                                dataType = symDataType;
                                dataTypes.insert(dataType);
                                return dataType;
                            }
                        }
                    }

                    // search for next list
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

                m_structListTypes.push_back(newStruct);

                // Add newStruct at right place in m_globals.
                int symbolPos;
                // if list element is transformed list or structure then this will allow add this list after it.
                if (listType->getElementType()->getTrueContainerDataType()->isStruct())
                {
                    symbolPos = m_globals->getSymbolPos(listType->getElementType()->getTrueContainerDataType()) + 1;
                }
                else
                {
                    // list <base types> will be at the beginning of structures declarations.
                    // This is will sort them in order as they were used.
                    static int s_symbolPos = 0;
                    symbolPos = s_symbolPos++;
                }

                // put new structure definition in globals before this structure
                m_globals->addSymbol(newStruct, symbolPos);

                dataType = newStruct;
                dataTypes.insert(dataType);
                break;
            }
        }
        case DataType::kStructType:
        {
            StructType *structType = dynamic_cast<StructType *>(dataType);
            assert(structType);

            if (isBinaryStruct(structType) || isListStruct(structType))
            {
                break;
            }

            for (StructMember *structMember : structType->getMembers())
            {
                setBinaryList(structMember);
                setNoSharedAnn(structType, structMember);
                structMember->setDataType(findChildDataType(dataTypes, structMember->getDataType()));
                structMember->setContainList(containsList(structMember->getDataType()));
                structMember->setContainString(containsString(structMember->getDataType()));
            }
            break;
        }
        case DataType::kUnionType:
        {
            // Keil need extra pragma option when unions are used.
            m_templateData["usedUnionType"] = true;
            UnionType *currentUnion = dynamic_cast<UnionType *>(dataType);
            assert(currentUnion);

            for (auto unionMember : currentUnion->getUnionMembers().getMembers())
            {
                setBinaryList(unionMember);
                setNoSharedAnn(currentUnion, unionMember);
                unionMember->setDataType(findChildDataType(dataTypes, unionMember->getDataType()));
            }
            break;
        }
        default:
        {
            break;
        }
    }

    return dataType;
}

void CGenerator::transformAliases()
{
    for (auto it : m_globals->getSymbolsOfType(DataType::kAliasTypeSymbol))
    {
        AliasType *aliasType = dynamic_cast<AliasType *>(it);
        assert(aliasType);

        std::set<DataType *> dataTypesNew;
        findChildDataType(dataTypesNew, aliasType);
    }
}

void CGenerator::setBinaryList(StructMember *structMember)
{
    DataType *dataType = structMember->getDataType();
    if (dataType->isBinary())
    {
        Annotation *listLength = structMember->findAnnotation(LENGTH_ANNOTATION);
        if (listLength)
        {
            BuiltinType *builtinType = dynamic_cast<BuiltinType *>(m_globals->getSymbol("uint8"));
            assert(builtinType);
            ListType *listType = new ListType(builtinType);
            structMember->setDataType(listType);
            listType->setLengthVariableName(listLength->getValueObject()->toString());
            m_listBinaryTypes.push_back(listType);
        }
    }
}

void CGenerator::generate()
{
    /* Generate file with shim code version. */
    m_templateData["versionGuardMacro"] = generateIncludeGuardName(format_string("erpc_generated_shim_code_crc_%d", m_idlCrc16).c_str());

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
    m_templateData["unions"] = empty;
    m_templateData["consts"] = empty;
    m_templateData["functions"] = empty;

    m_templateData["nonExternalStructUnion"] = false;

    // Keil need extra pragma option when unions are used.
    m_templateData["usedUnionType"] = false;

    /* Set directions constants*/
    m_templateData["InDirection"] = getDirection(kInDirection);
    m_templateData["OutDirection"] = getDirection(kOutDirection);
    m_templateData["InoutDirection"] = getDirection(kInoutDirection);
    m_templateData["ReturnDirection"] = getDirection(kReturn);

    parseSubtemplates();

    /* Generate file containing crc of IDL files. */
    generateCrcFile();

    transformAliases();

    // transform data types and populate groups symbol map with all symbol directions
    findGroupDataTypes();

    makeIncludesTemplateData();

    makeAliasesTemplateData();

    makeConstTemplateData();

    makeEnumsTemplateData();

    // for common header, only C specific
    makeSymbolsDeclarationTemplateData();

    // check if types header annotation is used
    if (m_def->hasProgramSymbol())
    {
        m_templateData["commonTypesFile"] = m_def->getProgramSymbol()->getAnnStringValue(TYPES_HEADER_ANNOTATION);
    }
    else
    {
        m_templateData["commonTypesFile"] = "";
    }

    for (Group *group : m_groups)
    {
        data_map groupTemplate;
        groupTemplate["name"] = group->getName();
        groupTemplate["includes"] = makeGroupIncludesTemplateData(group);
        groupTemplate["symbolsMap"] = makeGroupSymbolsTemplateData(group);
        groupTemplate["interfaces"] = makeGroupInterfacesTemplateData(group);
        group->setTemplate(groupTemplate);

        generateGroupOutputFiles(group);
    }

    // generate types header if used
    if (!m_templateData["commonTypesFile"]->getvalue().empty())
    {
        generateTypesHeaderFile();
    }
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
        if (!constVar->findAnnotation(EXTERNAL_ANNOTATION))
        {
            DataType *constVarType = dynamic_cast<DataType *>(constVar->getDataType());
            assert(constVarType);
            Value *constVarValue = constVar->getValue();

            if (nullptr == constVarValue)
            {
                throw semantic_error(format_string("line %d: Const pointing to null Value object.", constVar->getLastLine()).c_str());
            }

            /* Use char[] for constants. */
            if (constVarType->getTrueDataType()->isString())
            {
                constInfo["typeAndName"] = format_string("char %s [%d]", constVar->getOutputName().c_str(), constVarValue->toString().size() + 1);
            }
            else
            {
                constInfo["typeAndName"] = getTypenameName(constVarType, constVar->getOutputName());
            }
            constInfo["name"] = constVar->getOutputName();

            std::string value;
            if (constVarType->isEnum())
            {
                if (constVarValue->getType() != kIntegerValue)
                {
                    throw semantic_error(format_string("line %d: Const enum pointing to non-integer Value object.", constVar->getLastLine()).c_str());
                }

                EnumType *constEnum = dynamic_cast<EnumType *>(constVarType);
                assert(constEnum);
                for (EnumMember *enumMember : constEnum->getMembers())
                {
                    assert(dynamic_cast<IntegerValue *>(constVarValue));
                    if (enumMember->getValue() == dynamic_cast<IntegerValue *>(constVarValue)->getValue())
                    {
                        value = enumMember->getName();
                        break;
                    }
                }
                if (value.compare("") == 0)
                {
                    value = "(" + constVarType->getName() + ") " + constVarValue->toString();
                    Log::warning(format_string("Enum value '%s' is not poiting to any '%s' variable \n", constVarValue->toString().c_str(), constVarType->getName().c_str()).c_str());
                }
            }
            else
            {
                value = constVarValue->toString();
                if (kStringValue == constVarValue->getType())
                {
                    value = "\"" + value + "\"";
                }
            }
            constInfo["value"] = value;

            setTemplateComments(constVar, constInfo);
            Log::info("Name=%s\tType=%s\tValue=%s\n", constVar->getName().c_str(),
                      constVarType->getName().c_str(), constVar->getValue()->toString().c_str());
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
        if (!enumType->findAnnotation(EXTERNAL_ANNOTATION))
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
    enumInfo["name"] = enumType->getOutputName();
    enumInfo["members"] = getEnumMembersTemplateData(enumType);
    setTemplateComments(enumType, enumInfo);
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
        string memberDeclaration = member->getOutputName();
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
        setTemplateComments(member, enumMember);
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

    // All existing type declarations
    SymbolScope::symbol_vector_t aliasTypeVector = m_globals->getSymbolsOfType(DataType::kAliasTypeSymbol);

    /* type definions of structures */
    int i = 0;
    for (auto it : m_globals->getSymbolsOfType(DataType::kStructTypeSymbol))
    {
        StructType *structType = dynamic_cast<StructType *>(it);
        assert(structType);
        if (structType->getName().compare("") != 0 && !structType->findAnnotation(EXTERNAL_ANNOTATION))
        {
            AliasType *a = new AliasType(structType->getOutputName(), structType);
            aliasTypeVector.insert(aliasTypeVector.begin() + i++, a);
        }
    }

    /* type definions of non-encapsulated unions */
    for (auto it : m_globals->getSymbolsOfType(DataType::kUnionTypeSymbol))
    {
        UnionType *unionType = dynamic_cast<UnionType *>(it);
        assert(unionType);
        if (!unionType->findAnnotation(EXTERNAL_ANNOTATION))
        {
            AliasType *a = new AliasType(unionType->getOutputName(), unionType);
            aliasTypeVector.insert(aliasTypeVector.begin() + i++, a);
        }
    }

    /* type definitions of functions and table of functions */
    data_list functions;
    for (Symbol *functionTypeSymbol : m_globals->getSymbolsOfType(Symbol::kFunctionTypeSymbol))
    {
        FunctionType *functionType = dynamic_cast<FunctionType *>(functionTypeSymbol);
        data_map functionInfo;

        // aware of external function definitions
        if (!functionType->findAnnotation(EXTERNAL_ANNOTATION))
        {
            AliasType *a = new AliasType(getFunctionPrototype(nullptr, functionType), functionType);
            a->setMlComment(functionType->getMlComment());
            a->setIlComment(functionType->getIlComment());

            /* Function type definition need be inserted after all parameters types definitions. */
            DataType *callbackParamType = nullptr;
            for (StructMember *callbackParam : functionType->getParameters().getMembers())
            {
                DataType *callbackParamDataType = callbackParam->getDataType();
                if (!callbackParamType || callbackParamDataType->getFirstLine() > callbackParamType->getFirstLine())
                {
                    callbackParamType = callbackParamDataType;
                }
            }
            if (!callbackParamType || !callbackParamType->isAlias())
            {
                /* order isn't important */
                aliasTypeVector.insert(aliasTypeVector.begin() + i++, a);
            }
            else
            {
                /* skip structure, unions and functions type definitions */
                for (int aliasTypesIt = i; aliasTypesIt < aliasTypeVector.size(); ++aliasTypesIt)
                {
                    if (callbackParamType == aliasTypeVector[i])
                    {
                        aliasTypeVector.insert(aliasTypeVector.begin() + aliasTypesIt + 1, a);
                        break;
                    }
                }
            }
        }

        /* Table template data. */
        data_list callbacks;
        for (Function *fun : functionType->getCallbackFuns())
        {
            data_map callbacksInfo;
            callbacksInfo["name"] = fun->getName();
            callbacks.push_back(callbacksInfo);
        }
        functionInfo["callbacks"] = callbacks;
        /* Function type name. */
        functionInfo["name"] = functionType->getName();
        functions.push_back(functionInfo);
    }
    m_templateData["functions"] = functions;

    for (auto it : aliasTypeVector)
    {
        AliasType *aliasType = dynamic_cast<AliasType *>(it);
        assert(aliasType);
        if (!aliasType->findAnnotation(EXTERNAL_ANNOTATION))
        {
            Log::info("%d: ", n);
            data_map aliasInfo;
            DataType *elementDataType = aliasType->getElementType();
            setTemplateComments(aliasType, aliasInfo);

            if (elementDataType->getName() != "")
            {
                std::string realType;
                if (elementDataType->isFunction())
                {
                    realType = aliasType->getOutputName();
                    aliasInfo["name"] = elementDataType->getName();
                }
                else
                {
                    realType = getTypenameName(elementDataType, aliasType->getOutputName());
                    aliasInfo["name"] = aliasType->getOutputName();
                }

                Log::info("%s\n", realType.c_str());

                /* For case typedef struct/union */
                if (elementDataType->getName() == aliasType->getName() || elementDataType->getOutputName() == aliasType->getName())
                {
                    if (elementDataType->isStruct())
                    {
                        realType = "struct " + realType;
                    }
                    else
                    {
                        realType = "union " + realType;
                    }
                }

                aliasInfo["typenameName"] = realType;
            }
            else
            {
                aliasInfo["typenameName"] = "";
                aliasInfo["unnamedName"] = aliasType->getOutputName();
                switch (elementDataType->getDataType())
                {
                    case DataType::kStructType:
                    {
                        StructType *structType = dynamic_cast<StructType *>(elementDataType);
                        assert(structType);
                        aliasInfo["unnamed"] = getStructDefinitionTemplateData(nullptr, structType, getStructDeclarationTemplateData(structType));
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
    return (aliasType != nullptr) ? aliasType->getOutputName() : "";
}

void CGenerator::makeSymbolsDeclarationTemplateData()
{
    Log::info("Symbols templates:\n");

    for (auto it = m_globals->begin(); it != m_globals->end(); ++it)
    {
        data_map info;

        switch ((*it)->getSymbolType())
        {
            case DataType::kStructTypeSymbol:
            {
                StructType *structType = dynamic_cast<StructType *>(*it);
                assert(structType);

                info = getStructDeclarationTemplateData(structType);

                Log::info("%s\n", structType->getDescription().c_str());
                m_symbolsTemplate.push_back(info);
                break;
            }

            case DataType::kUnionTypeSymbol:
            {
                UnionType *unionType = dynamic_cast<UnionType *>(*it);
                assert(unionType);

                info = getUnionDeclarationTemplateData(unionType);

                Log::info("%s\n", unionType->getDescription().c_str());
                m_symbolsTemplate.push_back(info);
                break;
            }

            default:
                break;
        }
    }

    // header should have all data type defines
    m_templateData["symbols"] = m_symbolsTemplate;
}

data_map CGenerator::getSymbolTemplateByName(const std::string &name)
{
    for (data_ptr symbol : m_symbolsTemplate)
    {
        if (symbol->getmap()["name"]->getvalue() == name)
        {
            return symbol->getmap();
        }
    }

    data_map empty;
    return empty;
}

data_map CGenerator::makeGroupSymbolsTemplateData(Group *group)
{
    data_map symbolsTemplate;
    std::set<std::string> names;

    data_list symbolsToClient;
    data_list symbolsToServer;
    data_list symbolsServerFree;

    Log::info("Group symbols:\n");

    for (Symbol *symbol : group->getSymbols())
    {
        data_map info;
        const std::set<_param_direction> dirs = group->getSymbolDirections(symbol);
        if (dirs.size())
        {
            switch (symbol->getSymbolType())
            {
                case DataType::kStructTypeSymbol:
                {
                    StructType *structType = dynamic_cast<StructType *>(symbol);
                    assert(structType);

                    Log::info("%s\n", structType->getDescription().c_str());

                    std::string name = (structType->getName() != "") ? structType->getOutputName() : getAliasName(structType);

                    // check if template for this structure has not already been generated
                    if (names.find(name) == names.end())
                    {
                        // get struct declaration info
                        info = getSymbolTemplateByName(name);
                        if (info.empty())
                        {
                            break;
                        }

                        // get struct definition info
                        info = getStructDefinitionTemplateData(group, structType, info);

                        // set symbol data to client or server according to its directions
                        setSymbolDataToSide(structType, dirs, symbolsToClient, symbolsToServer, info);

                        // struct needs to be freed?
                        set<DataType *> loopDetection;
                        if (structType->containStringMember() || structType->containListMember() || containsByrefParamToFree(structType, loopDetection))
                        {
                            symbolsServerFree.push_back(info);
                        }

                        names.insert(name);
                    }
                    break;
                }
                case DataType::kUnionTypeSymbol:
                {
                    UnionType *unionType = dynamic_cast<UnionType *>(symbol);
                    assert(unionType);

                    Log::info("%s\n", unionType->getDescription().c_str());

                    std::string name = unionType->getOutputName();

                    // check if template for this union has not already been generated
                    if (names.find(name) == names.end())
                    {
                        info = getSymbolTemplateByName(name);
                        if (info.empty())
                        {
                            break;
                        }

                        // get union info
                        data_map unionBaseInfo = getUnionDeclarationTemplateData(unionType);
                        bool needUnionsServerFree = false;
                        info = getUnionDefinitionTemplateData(group, unionType, unionBaseInfo, needUnionsServerFree);

                        // set symbol data to client or server according to its directions
                        setSymbolDataToSide(unionType, dirs, symbolsToClient, symbolsToServer, info);

                        // free unions on server side.
                        if (needUnionsServerFree)
                        {
                            symbolsServerFree.push_back(info);
                        }

                        names.insert(name);
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }

    symbolsTemplate["symbolsToClient"] = symbolsToClient;
    symbolsTemplate["symbolsToServer"] = symbolsToServer;
    symbolsTemplate["symbolsServerFree"] = symbolsServerFree;

    return symbolsTemplate;
}

data_map CGenerator::getStructDeclarationTemplateData(StructType *structType)
{
    data_map info;
    info["name"] = (structType->getName() != "" ? structType->getOutputName() : getAliasName(structType));
    info["type"] = "struct";

    setTemplateComments(structType, info);

    // external annotation
    if (structType->findAnnotation(EXTERNAL_ANNOTATION))
    {
        info["isExternal"] = true;
    }
    else
    {
        info["isExternal"] = false;
        m_templateData["nonExternalStructUnion"] = true;
    }

    data_list members;
    for (auto member : structType->getMembers())
    {
        data_map member_info;

        std::string memberName = member->getOutputName();
        member_info["name"] = memberName;

        DataType *trueDataType = member->getDataType()->getTrueDataType();
        //Check if member is byRef type. Add "*" for type and allocate space for data on server side.
        if (member->isByref() && (trueDataType->isStruct() || trueDataType->isUnion() || trueDataType->isScalar() || trueDataType->isEnum()))
        {
            memberName = "*" + memberName;
        }
        member_info["memberDeclaration"] = getTypenameName(member->getDataType(), memberName) + ";";

        member_info["elementsCount"] = "";
        if (isBinaryStruct(structType))
        {
            member_info["elementsCount"] = "uint32_t dataLength;";
        }
        else if (isListStruct(structType))
        {
            member_info["elementsCount"] = "uint32_t elementsCount;";
        }

        setTemplateComments(member, member_info);

        members.push_back(member_info);
    }
    info["members"] = members;

    return info;
}

data_map CGenerator::getStructDefinitionTemplateData(Group *group, StructType *structType, data_map structInfo)
{
    structInfo["hasNullableMember"] = false;
    structInfo["needTempVariable"] = false;
    structInfo["genStructWrapperF"] = !isBinaryStruct(structType);
    structInfo["noSharedMem"] = (structType->findAnnotation(NO_SHARED_ANNOTATION) != nullptr);

    setTemplateComments(structType, structInfo);

    // set struct members template data
    data_list members;
    data_list baseMembers = dynamic_cast<DataList *>(structInfo["members"].get().get())->getlist();
    data_list membersToFree;

    for (StructMember *member : structType->getMembers())
    {
        data_map member_info;

        // find base member template
        for (data_ptr mt : baseMembers)
        {
            if (mt->getmap()["name"]->getvalue() == member->getOutputName())
            {
                member_info = mt->getmap();
                break;
            }
        }
        if (member_info.empty())
        {
            Log::info("Could not find base member template for member '%s'\n", member->getName().c_str());
            continue;
        }

        // member data type
        DataType *dataType = member->getDataType();
        DataType *trueDataType = member->getDataType()->getTrueDataType();
        std::string memberName = member->getOutputName();

        bool isPtrType = (member->isByref() || trueDataType->isBinary() || trueDataType->isString() || trueDataType->isList() || trueDataType->isFunction());
        // When forward declaration is used member should be present as pointer to data.
        if (!isPtrType && isMemberDataTypeUsingForwardDeclaration(structType, dataType))
        {
            throw syntax_error(format_string("line %d: Struct member shall use byref option. Member is using forward declared type.", member->getFirstLine()).c_str());
        }
        // Handle nullable annotation
        bool isNullable = ((member->findAnnotation(NULLABLE_ANNOTATION) != nullptr) && (isPtrType || (trueDataType->isStruct() && (isListStruct(dynamic_cast<StructType *>(trueDataType)) || (isBinaryStruct(dynamic_cast<StructType *>(trueDataType)))))));
        member_info["isNullable"] = isNullable;
        member_info["structElements"] = "";
        member_info["structElementsCount"] = "";
        member_info["noSharedMem"] = (member->findAnnotation(NO_SHARED_ANNOTATION) != nullptr);

        if (isNullable)
        {
            // hasNullableMember must be true if there at least one struct member with nullable annotation
            structInfo["hasNullableMember"] = true;

            // Set a flag on the struct indicating there are nullable members, but only
            // set it to true
            if (dataType->isStruct() && (isListStruct(dynamic_cast<StructType *>(dataType)) ||
                                         isBinaryStruct(dynamic_cast<StructType *>(dataType))))
            {
                if (isListStruct(dynamic_cast<StructType *>(dataType)))
                {
                    member_info["structElements"] = ".elements";
                    member_info["structElementsCount"] = ".elementsCount";
                }
                else if (isBinaryStruct(dynamic_cast<StructType *>(dataType)))
                {
                    member_info["structElements"] = ".data";
                    member_info["structElementsCount"] = ".dataLength";
                }
            }
        }

        // Skip data serialization for variables placed as @length value for lists.
        // Skip data serialization for variables used as discriminator for unions.
        // These prevent to serialized data twice.
        StructMember *referencedFrom = findParamReferencedFrom(structType->getMembers(), member->getName());
        if (referencedFrom && !member->findAnnotation(SHARED_ANNOTATION))
        {
            Log::debug("Skipping EncodeDecode member '%s' with paramType '%s' (it's serialized with member '%s').\n", memberName.c_str(), dataType->getName().c_str(), referencedFrom->getName().c_str());
            member_info["coderCall"] = "";
            member_info["serializedViaMember"] = referencedFrom->getOutputName();
        }
        else
        {
            Log::debug("Calling EncodeDecode member '%s' with paramType '%s'.\n", memberName.c_str(), dataType->getName().c_str());

            // Subtemplate setup for read/write struct calls
            bool needTempVariable = false;
            member_info["coderCall"] = getEncodeDecodeCall("data->" + memberName, group, dataType, structType, true, member, needTempVariable, false);

            if (needTempVariable)
            {
                structInfo["needTempVariable"] = true;
            }

            member_info["serializedViaMember"] = "";
        }

        members.push_back(member_info);
        if (member->isByref() || isNeedCallFree(dataType))
        {
            membersToFree.push_back(member_info);
        }
    }

    structInfo["members"] = members; // overwrite member's declaration with whole definition
    structInfo["membersToFree"] = membersToFree;

    return structInfo;
}

data_map CGenerator::getUnionDeclarationTemplateData(UnionType *unionType)
{
    data_map info;
    info["name"] = unionType->getOutputName();
    info["type"] = "union";

    setTemplateComments(unionType, info);
    // external annotation
    if (unionType->findAnnotation(EXTERNAL_ANNOTATION))
    {
        info["isExternal"] = true;
    }
    else
    {
        info["isExternal"] = false;
        m_templateData["nonExternalStructUnion"] = true;
    }

    setUnionMembersTemplateData(unionType, info);

    return info;
}

data_map CGenerator::getUnionDefinitionTemplateData(Group *group, UnionType *unionType, cpptempl::data_map &unionInfo, bool &needUnionsServerFree)
{
    bool needTempVariable = false;
    unionInfo["coderCall"] = getEncodeDecodeCall("data->", nullptr, unionType, nullptr, true, nullptr, needTempVariable, false);
    unionInfo["needTempVariable"] = needTempVariable;

    unionInfo["noSharedMem"] = (unionType->findAnnotation(NO_SHARED_ANNOTATION) != nullptr);

    unionInfo["membersData"] = getUnionMembersData(unionType, "");
    // free unions on server side.
    for (auto unionCase : unionType->getUniqueCases())
    {
        if (!unionCase->caseMemberIsVoid())
        {
            for (auto memberName : unionCase->getMemberDeclarationNames())
            {
                StructMember *unionMember = unionCase->getUnionMemberDeclaration(memberName);
                DataType *trueMemberDataType = unionMember->getDataType()->getTrueDataType();
                if (unionMember->isByref() || trueMemberDataType->isList() || trueMemberDataType->isBinary() || trueMemberDataType->isString())
                {
                    needUnionsServerFree = true;
                    break;
                }
                else if (trueMemberDataType->isStruct())
                {
                    StructType *structType = dynamic_cast<StructType *>(trueMemberDataType);
                    assert(structType);
                    set<DataType *> loopDetection;
                    if (structType->containStringMember() || structType->containListMember() || containsByrefParamToFree(structType, loopDetection))
                    {
                        needUnionsServerFree = true;
                        break;
                    }
                }
            }
        }

        if (needUnionsServerFree)
        {
            break;
        }
    }
    return unionInfo;
}

void CGenerator::setUnionMembersTemplateData(UnionType *unionType, cpptempl::data_map &unionInfo)
{
    data_list unionCasesList;
    for (auto unionCase : unionType->getUniqueCases())
    {
        if (!unionCase->caseMemberIsVoid())
        {
            data_list unionCaseList;
            for (auto memberName : unionCase->getMemberDeclarationNames())
            {
                data_map unionMemberMap;
                StructMember *unionMember = unionCase->getUnionMemberDeclaration(memberName);
                std::string unionMemberName = unionMember->getOutputName();
                DataType *unionMemberType = unionMember->getDataType()->getTrueDataType();

                //Check if member is byRef type. Add "*" for type and allocate space for data on server side.
                if (unionMember->isByref() && (unionMemberType->isStruct() || unionMemberType->isUnion() || unionMemberType->isScalar() || unionMemberType->isEnum()))
                {
                    unionMemberName = "*" + unionMemberName;
                }

                unionMemberMap["typenameName"] = getTypenameName(unionMember->getDataType(), unionMemberName);
                unionCaseList.push_back(unionMemberMap);
            }
            unionCasesList.push_back(unionCaseList);
        }
    }
    unionInfo["unionCases"] = unionCasesList;
}

std::string CGenerator::getUnionMembersData(UnionType *unionType, std::string ident)
{
    std::string returnName;
    /* Get union cases declaration description. */
    for (auto unionCase : unionType->getUniqueCases())
    {
        if (!unionCase->caseMemberIsVoid())
        {
            std::string caseBegin;
            std::string caseEnd;
            std::string caseIdent = ident + "    ";
            if (1 < unionCase->getMemberDeclarationNames().size())
            {
                caseBegin = ident + std::string("    ") + std::string("struct\n") + ident + std::string("    {\n");
                caseEnd = ident + std::string("    ") + std::string("};\n");
                caseIdent += "    ";
            }
            std::string returnCase;
            for (auto memberName : unionCase->getMemberDeclarationNames())
            {
                StructMember *unionMember = unionCase->getUnionMemberDeclaration(memberName);
                returnCase += caseIdent;
                std::string unionMemberName = unionMember->getOutputName();
                DataType *unionType = unionMember->getDataType()->getTrueDataType();
                if (unionMember->isByref() && (unionType->isStruct() || unionType->isUnion() || unionType->isScalar() || unionType->isEnum()))
                {
                    unionMemberName = "*" + unionMemberName;
                }
                returnCase += getTypenameName(unionMember->getDataType(), unionMember->getOutputName());
                returnCase += ";\n";
            }
            returnName += caseBegin + returnCase + caseEnd;
        }
    }
    return returnName;
}

void CGenerator::setTemplateComments(Symbol *symbol, cpptempl::data_map &symbolInfo)
{
    symbolInfo["mlComment"] = symbol->getMlComment();
    symbolInfo["ilComment"] = symbol->getIlComment();
}
bool CGenerator::isServerNullParam(StructMember *param)
{
    DataType *paramTrueDataType = param->getDataType()->getTrueDataType();
    return (!paramTrueDataType->isScalar() && !paramTrueDataType->isEnum() && !paramTrueDataType->isArray());
}

bool CGenerator::isPointerParam(StructMember *param)
{
    DataType *paramTrueDataType = param->getDataType()->getTrueDataType();
    return (isServerNullParam(param) || ((paramTrueDataType->isScalar() || paramTrueDataType->isEnum()) && param->getDirection() != kInDirection));
}

bool CGenerator::isNullableParam(StructMember *param)
{
    return (param->findAnnotation(NULLABLE_ANNOTATION) && isPointerParam(param));
}

data_map CGenerator::getFunctionTemplateData(Group *group, Function *fn, int fnIndex)
{
    data_map info;
    std::string proto = getFunctionPrototype(group, fn);
    std::string serverProto = getFunctionServerCall(fn);

    info["name"] = fn->getOutputName();
    info["prototype"] = proto;
    info["serverPrototype"] = serverProto;
    info["id"] = fn->getUniqueId();
    info["isOneway"] = fn->isOneway();
    info["isReturnValue"] = !fn->isOneway();
    info["isSendValue"] = false;
    setTemplateComments(fn, info);
    info["needTempVariableServer"] = false;
    info["needTempVariableClient"] = false;
    info["needNullVariableOnServer"] = false;

    /* Is function declared as external? */
    info["isNonExternalFunction"] = !fn->findAnnotation(EXTERNAL_ANNOTATION);

    // Get return value info
    data_map returnInfo;
    returnInfo["type"] = getTypeInfo(fn->getReturnType(), true);
    StructMember *structMember = fn->getReturnStructMemberType();
    DataType *dataType = fn->getReturnType();
    DataType *trueDataType = dataType->getTrueDataType();
    if (!trueDataType->isVoid())
    {
        std::string result = "result";
        bool needTempVariable = false;
        setCallingFreeFunctions(structMember, returnInfo, true);
        std::string extraPointer = getExtraPointerInReturn(dataType);
        std::string resultVariable = extraPointer + returnSpaceWhenNotEmpty(extraPointer) + result;
        if (structMember->findAnnotation(NULLABLE_ANNOTATION) == nullptr)
        {
            returnInfo["isNullable"] = false;
            returnInfo["nullableName"] = "";
        }
        else
        {
            returnInfo["isNullable"] = true;
            returnInfo["nullableName"] = result;
        }
        bool isShared = (structMember->findAnnotation(SHARED_ANNOTATION) != nullptr);
        if (!isShared)
        {
            returnInfo["firstAlloc"] = firstAllocOnReturnWhenIsNeed(result, dataType);
            result = extraPointer + result;
        }
        else
        {
            returnInfo["firstAlloc"] = "";
        }

        // due to compatibility with function parameters.
        returnInfo["lengthName"] = "";
        returnInfo["nullVariable"] = "";

        returnInfo["direction"] = getDirection(kReturn);
        returnInfo["coderCall"] = getEncodeDecodeCall(result, group, dataType, nullptr, false, structMember, needTempVariable, true);
        returnInfo["shared"] = isShared;
        resultVariable = getTypenameName(dataType, resultVariable);
        info["needTempVariableClient"] = needTempVariable;
        returnInfo["resultVariable"] = resultVariable;
        returnInfo["errorReturnValue"] = getErrorReturnValue(fn);
        returnInfo["isNullReturnType"] =
            (!trueDataType->isScalar() && !trueDataType->isEnum());
    }
    info["returnValue"] = returnInfo;

    // get function parameter info
    auto fnParams = fn->getParameters().getMembers();
    data_list params;
    data_list paramsToFree;
    data_list paramsToClient;
    data_list paramsToServer;
    for (StructMember *param : fnParams)
    {
        data_map paramInfo;
        bool needTempVariable = false;
        DataType *paramType = param->getDataType();
        DataType *paramTrueType = paramType->getTrueDataType();
        std::string name = param->getOutputName();

        // Init parameters to NULL on server side
        bool isServerNull = isServerNullParam(param);
        paramInfo["isNullParam"] = isServerNull;

        // Handle nullable annotation.
        bool isNullable = isNullableParam(param);
        paramInfo["isNullable"] = isNullable;

        // Serialize when list/union is nullable
        paramInfo["referencedName"] = "";

        // Skip data serialization for variables placed as @length value for lists.
        // Skip data serialization for variables used as discriminator for unions.
        // These prevent to serialized data twice.
        StructMember *referencedFrom = findParamReferencedFrom(fnParams, param->getName());
        if (referencedFrom != nullptr && referencedFrom->findAnnotation(SHARED_ANNOTATION) == nullptr)
        {
            bool isNullableReferenced = isNullableParam(referencedFrom);
            if (isNullable && !isNullableReferenced)
            {
                Log::error(
                    "line %d: Param1 '%s' is serialized through param2 '%s'. Annotation @nullable can be applied for "
                    "param1 only when same annotation is applied for param2.\n",
                    param->getFirstLine(), param->getName().c_str(), referencedFrom->getName().c_str());
                paramInfo["isNullable"] = false;
            }

            // Serialize when list/union is nullable
            if (isNullableReferenced)
            {
                paramInfo["referencedName"] = referencedFrom->getOutputName();
            }

            // Directions in which list/union is serializing reference
            if (referencedFrom->getDirection() == param->getDirection())
            {
                paramInfo["serializedDirection"] = getDirection(kInoutDirection);
            }
            else
            {
                paramInfo["serializedDirection"] = getDirection(referencedFrom->getDirection());
            }
        }
        else
        {
            paramInfo["serializedDirection"] = "";
        }

        // data == NULL also when length variable == NULL
        paramInfo["lengthName"] = "";

        // due to compatibility with return data.
        paramInfo["firstAlloc"] = "";

        // Need extra variable to handle nullable for enums and scalar types on server side.
        paramInfo["nullVariable"] = "";
        if (isNullable)
        {
            // Out @nullable param need send @nullable information
            info["isSendValue"] = true;

            // data == NULL also when length variable == NULL
            string lengthParam = param->getAnnStringValue(LENGTH_ANNOTATION);
            if (!lengthParam.empty())
            {
                Symbol *symbol = fn->getParameters().getScope().getSymbol(lengthParam, false);
                if (symbol != nullptr)
                {
                    StructMember *st = dynamic_cast<StructMember *>(symbol);
                    assert(st);
                    if (isNullableParam(st))
                    {
                        paramInfo["lengthName"] = symbol->getOutputName();
                    }
                }
            }

            // Special case when scalar variables are @nullable
            std::string nullableName = param->getOutputName();
            paramInfo["nullableName"] = nullableName;
            if (paramTrueType->isScalar() || paramTrueType->isEnum())
            {
                std::string nullVariable = "_" + nullableName;
                paramInfo["nullVariable"] = getTypenameName(paramTrueType, "*_" + nullableName);
            }

            // Set flags to indicate whether a local isNull variable is needed on the
            // server and client sides.
            //
            // If needNullVariableX is true, we don't want to overwrite it to false
            // if a later parameter is not nullable. So, we will only try to set it
            // if it is not true. Once the variable's value is true, we know we need
            // a null variable at least once.
            info["needNullVariableOnServer"] = true;
        }

        // Check if max_length annotation belongs to "in" param
        // TODO: Should be global, also for PythonGenerator
        if (paramTrueType->isList() || paramTrueType->isString())
        {
            std::string maxLengthName = param->getAnnStringValue(MAX_LENGTH_ANNOTATION);
            if (!maxLengthName.empty())
            {
                Symbol *symbol = fn->getParameters().getScope().getSymbol(maxLengthName, false);
                if (symbol)
                {
                    StructMember *structMember = dynamic_cast<StructMember *>(symbol);
                    assert(structMember);
                    if (structMember->getDirection() != kInDirection)
                    {
                        throw semantic_error(
                            format_string("line %d, ref %d: The parameter named by a max_length annotation must be "
                                          "'in' direction type.",
                                          param->getLocation().m_firstLine, structMember->getLocation().m_firstLine));
                    }
                }
            }
        }

        paramInfo["mallocServer"] = firstAllocOnServerWhenIsNeed(name, param);
        setCallingFreeFunctions(param, paramInfo, false);

        // Use shared memory feature instead of serializing/deserializing data.
        bool isShared = (isPointerParam(param) && param->findAnnotation(SHARED_ANNOTATION) != nullptr);
        paramInfo["shared"] = isShared;
        std::string encodeDecodeName;
        if (isShared)
        {
            encodeDecodeName = name;
            if (!paramTrueType->isList())
            {
                name = "*" + name;
            }
        }
        else
        {
            encodeDecodeName = name = getExtraDirectionPointer(param) + name;
        }

        std::string variable = getTypenameName(paramType, name);
        paramInfo["variable"] = variable;
        paramInfo["name"] = name;
        paramInfo["type"] = getTypeInfo(paramType, false);

        Log::debug("Calling EncodeDecode param %s with paramType %s.\n", param->getName().c_str(), paramType->getName().c_str());
        paramInfo["coderCall"] = getEncodeDecodeCall(encodeDecodeName, group, paramType, &fn->getParameters(), false, param, needTempVariable, true);

        // set parameter direction
        paramInfo["direction"] = getDirection(param->getDirection());

        setSymbolDataToSide(param, group->getSymbolDirections(param), paramsToClient, paramsToServer, paramInfo);

        if (needTempVariable && param->getDirection() != kInDirection)
        {
            info["needTempVariableClient"] = true;
        }
        if (needTempVariable && (param->getDirection() == kInDirection || param->getDirection() == kInoutDirection))
        {
            info["needTempVariableServer"] = true;
        }
        params.push_back(paramInfo);

        // Generating top of freeing functions in generated output.
        bool l_generateServerFunctionParamFreeFunctions = (!isShared && generateServerFreeFunctions(param));
        if (l_generateServerFunctionParamFreeFunctions &&
            (isNeedCallFree(paramType) || paramInfo["firstFreeingCall1"]->getmap()["freeName"]->getvalue() != "" ||
             paramInfo["firstFreeingCall2"]->getmap()["freeName"]->getvalue() != ""))
        {
            paramsToFree.push_back(paramInfo);
        }
    }
    if (paramsToClient.size() > 0)
    {
        info["isReturnValue"] = true;
    }
    if (paramsToServer.size() > 0)
    {
        info["isSendValue"] = true;
    }
    info["parameters"] = make_data(params);
    info["paramsToFree"] = paramsToFree;
    info["parametersToClient"] = paramsToClient;
    info["parametersToServer"] = paramsToServer;

    Log::info("    %d: (%d) %s\n", fnIndex, fn->getUniqueId(), info["prototype"]->getvalue().c_str());

    return info;
}

void CGenerator::setSymbolDataToSide(const Symbol *symbolType, const std::set<_param_direction> directions, data_list &toClient, data_list &toServer, data_map &dataMap)
{
    _direction direction = kIn;
    switch (symbolType->getSymbolType())
    {
        case Symbol::kStructTypeSymbol:
        case Symbol::kUnionTypeSymbol:
        case Symbol::kFunctionTypeSymbol:
        {
            bool in = directions.count(kInDirection);
            bool out = directions.count(kOutDirection);
            bool outByref = directions.count(kOutDirectionByref);
            bool inOut = directions.count(kInoutDirection);
            bool ret = directions.count(kReturn);

            Log::info("Symbol %s has directions: in:%d, out:%d, outByRef:%d, inOut:%d, ret:%d\n", symbolType->getName().c_str(), in, out, outByref, inOut, ret);

            if (inOut || (in && (outByref || ret || out)))
            {
                direction = kInOut;
            }
            else if (outByref || ret || out)
            {
                direction = kOut;
            }
            else if (!in && !out && !ret && !inOut)
            {
                // ToDo: shared pointer.
                direction = kNone;
            }

            break;
        }
        case Symbol::kStructMemberSymbol:
        {
            const StructMember *structMember = dynamic_cast<const StructMember *>(symbolType);
            assert(structMember);
            switch (structMember->getDirection())
            {
                case kOutDirection:
                case kOutDirectionByref:
                {
                    direction = kOut;
                    break;
                }
                case kInoutDirection:
                {
                    direction = kInOut;
                    break;
                }
                case kInDirection:
                {
                    direction = kIn;
                    break;
                }
                default:
                {
                    throw internal_error("Unsupported direction type of structure member.");
                }
            }
            break;
        }
        default:
        {
            throw internal_error(
                format_string("Symbol: %s is not structure or function parameter.", symbolType->getDescription().c_str()));
        }
    }

    switch (direction)
    {
        case kIn:
        {
            toServer.push_back(dataMap);
            break;
        }
        case kOut:
        {
            toClient.push_back(dataMap);
            break;
        }
        case kInOut:
        {
            toServer.push_back(dataMap);
            toClient.push_back(dataMap);
            break;
        }
        case kNone: // ToDo: shared pointer
        {
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
    Value *returnVal = fn->getAnnValue(ERROR_RETURN_ANNOTATION);
    DataType *dataType = fn->getReturnType()->getTrueDataType();
    if (returnVal)
    {
        if (returnVal->toString().empty())
        {
            throw semantic_error(format_string("Expected value for @%s annotation for function on line %d", ERROR_RETURN_ANNOTATION,
                                               fn->getLocation().m_firstLine));
        }
        else if (dataType->isString())
        {
            return "(char *) " + returnVal->toString();
        }
        else if (dataType->isScalar())
        {
            BuiltinType *builtinType = dynamic_cast<BuiltinType *>(dataType);
            assert(builtinType);
            switch (builtinType->getBuiltinType())
            {
                case BuiltinType::kUInt8Type:
                case BuiltinType::kUInt16Type:
                case BuiltinType::kUInt32Type:
                case BuiltinType::kUInt64Type:
                {
                    IntegerValue *integerValue = dynamic_cast<IntegerValue *>(returnVal);
                    return format_string("%lu", integerValue->getValue());
                }
                default:
                    return returnVal->toString();
            }
        }
        return returnVal->toString();
    }
    else
    {
        if (dataType->isScalar())
        {
            BuiltinType *builtinType = dynamic_cast<BuiltinType *>(dataType);
            assert(builtinType);
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
            return "(" + fn->getReturnType()->getOutputName() + ") -1";
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
    proto += fn->getOutputName();
    proto += "(";

    auto params = fn->getParameters().getMembers();

    if (params.size())
    {
        int n = 0;
        for (auto it : params)
        {
            bool isLast = (n == params.size() - 1);
            DataType *trueDataType = it->getDataType()->getTrueDataType();

            /* Builtin types and function types. */
            if (((trueDataType->isScalar()) || trueDataType->isEnum()) && it->getDirection() != kInDirection && it->findAnnotation(NULLABLE_ANNOTATION))
            {
                //On server side is created new variable for handle null : "_" + name
                proto += "_";
            }
            else if ((it->getDirection() != kInDirection) && (((trueDataType->isScalar()) || trueDataType->isEnum()) ||
                                                              (it->findAnnotation(SHARED_ANNOTATION))))

            {
                proto += "&";
            }
            proto += it->getOutputName();

            if (!isLast)
            {
                proto += ", ";
            }
            ++n;
        }
    }
    return proto + ");";
}

std::string CGenerator::getFunctionPrototype(Group *group, FunctionBase *fn)
{
    DataType *dataTypeReturn = fn->getReturnType();
    std::string proto = getExtraPointerInReturn(dataTypeReturn);
    if (proto == "*")
    {
        proto += " ";
    }

    Symbol *symbol = dynamic_cast<Symbol *>(fn);
    assert(symbol);
    std::string functionName = symbol->getOutputName();

    FunctionType *funType = dynamic_cast<FunctionType *>(fn);
    if (funType) /* Need add '(*name)' for function type definition. */
    {
        proto += "(*" + functionName + ")";
    }
    else /* Use function name only. */
    {
        proto += functionName;
    }

    proto += "(";

    auto params = fn->getParameters().getMembers();
    if (params.size())
    {
        int n = 0;
        for (auto it : params)
        {
            bool isLast = (n == params.size() - 1);
            std::string paramSignature = it->getOutputName();
            DataType *dataType = it->getDataType();
            DataType *trueDataType = dataType->getTrueDataType();

            /* Add '*' to data types. */
            if (((trueDataType->isBuiltin() || trueDataType->isEnum()) &&
                 (it->getDirection() != kInDirection && !trueDataType->isString())) ||
                (trueDataType->isFunction() && (it->getDirection() == kOutDirection || it->getDirection() == kInoutDirection)))
            {
                paramSignature = "* " + paramSignature;
            }
            else
            {
                std::string directionPointer = getExtraDirectionPointer(it);
                paramSignature = directionPointer + returnSpaceWhenNotEmpty(directionPointer) + paramSignature;
            }
            paramSignature = getTypenameName(dataType, paramSignature);

            if (!(m_def->hasProgramSymbol() && m_def->getProgramSymbol()->findAnnotation(NO_CONST_PARAM)))
            {
                if ((dataType->isString() || dataType->isFunction() || trueDataType->isStruct() || trueDataType->isUnion()) && it->getDirection() == kInDirection)
                {
                    proto += "const ";
                }
            }

            DataType *trueContainerDataType = dataType->getTrueContainerDataType();
            if (trueContainerDataType->isStruct())
            {
                StructType *structType = dynamic_cast<StructType *>(trueContainerDataType);
                assert(structType);
                //Todo: Need check if members are/aren't shared.
                if (group != nullptr)
                {
                    const std::set<_param_direction> directions = group->getSymbolDirections(structType);
                    if (!it->findAnnotation(SHARED_ANNOTATION) && (directions.count(kInoutDirection) &&
                                                                   (directions.count(kOutDirection) ||
                                                                    directions.count(kOutDirectionByref) ||
                                                                    directions.count(kReturn))))
                    {
                        throw syntax_error(
                            format_string("line %d: structs, lists, and binary cannot be used as both "
                                          "inout and out parameters in the same application",
                                          it->getLocation().m_firstLine));
                    }
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
        case DataType::kUnionType:
        {
            UnionType *unionType = dynamic_cast<UnionType *>(t);
            assert(unionType);
            if (unionType->isNonEncapsulatedUnion())
            {
                returnName = t->getOutputName();
                returnName += returnSpaceWhenNotEmpty(name) + name;
            }
            else
            {
                returnName += "union\n    {\n";
                returnName += getUnionMembersData(unionType, "    ");
                returnName += "    } " + name;
            }
            break;
        }
        case DataType::kVoidType:
        {
            returnName = "void";
            returnName += returnSpaceWhenNotEmpty(name) + name;
            break;
        }
        case DataType::kAliasType:
        case DataType::kEnumType:
        case DataType::kFunctionType:
        case DataType::kStructType:
        {
            returnName = t->getOutputName();
            returnName += returnSpaceWhenNotEmpty(name) + name;
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

void CGenerator::getEncodeDecodeBuiltin(Group *group,
                                        BuiltinType *t,
                                        data_map &templateData,
                                        StructType *structType,
                                        StructMember *structMember,
                                        bool isFunctionParam)
{
    templateData["decode"] = m_templateData["decodeBuiltinType"];
    templateData["encode"] = m_templateData["encodeBuiltinType"];
    if (t->isString())
    {
        templateData["checkStringNull"] = false;
        templateData["withoutAlloc"] = ((structMember && structMember->getDirection() == kInoutDirection) ||
                                        (structType && group->getSymbolDirections(structType).count(kInoutDirection))) ?
                                           true :
                                           false;
        if (!isFunctionParam)
        {
            templateData["stringAllocSize"] = structMember->getOutputName() + "_len";
            templateData["stringLocalName"] = structMember->getOutputName();
        }
        else
        {
            if (!structType)
            {
                templateData["stringAllocSize"] = "return_len";
                templateData["stringLocalName"] = "return";
            }
            else
            {
                templateData["checkStringNull"] = true;
                templateData["stringLocalName"] = structMember->getOutputName();
                templateData["stringAllocSize"] = structMember->getAnnStringValue(MAX_LENGTH_ANNOTATION);
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

data_map CGenerator::getEncodeDecodeCall(const std::string &name,
                                         Group *group,
                                         DataType *t,
                                         StructType *structType,
                                         bool inDataContainer,
                                         StructMember *structMember,
                                         bool &needTempVariable,
                                         bool isFunctionParam)
{
    // prepare data for template
    data_map templateData;
    string_vector params = string_vector(1, "");
    templateData["freeingCall"] = make_template("", &params);
    templateData["inDataContainer"] = inDataContainer;
    templateData["isElementArrayType"] = false;
    templateData["memberAllocation"] = "";
    // name used for serializing/deserializing current data type.
    std::string localName;

    int pos = name.rfind("*");
    if ((pos == 0 || pos == 1) && (t->getTrueDataType()->isStruct() || t->getTrueDataType()->isUnion()) && inDataContainer == false)
    {
        localName = name.substr(1, name.length());
    }
    else
    {
        localName = name;
    }

    templateData["freeName"] = localName;

    /* Check if member should be serialized as shared. */
    if ((structMember && structMember->findAnnotation(SHARED_ANNOTATION)) || (structType && structType->findAnnotation(SHARED_ANNOTATION)))
    {
        templateData["funcParam"] = (structType) ? true : false;
        templateData["InoutOutDirection"] = (structMember && (structMember->getDirection() == kOutDirection || structMember->getDirection() == kInoutDirection)) ? true : false;
        templateData["encode"] = m_templateData["encodeSharedType"];
        templateData["decode"] = m_templateData["decodeSharedType"];
        templateData["name"] = name;
        templateData["sharedTypeName"] = t->getName();

        /* If shared member contains non-shared member, it has to be serialized. */
        templateData["sharedType"] = "";
        if (t->isStruct() || t->isUnion())
        {
            StructType *s = dynamic_cast<StructType *>(t);
            if (s)
            {
                for (StructMember *m : s->getMembers())
                {
                    if (m->findAnnotation(NO_SHARED_ANNOTATION))
                    {
                        templateData["sharedType"] = "struct";
                        templateData["dataLiteral"] = (inDataContainer) ? "data->" : "";
                        break;
                    }
                }
            }
            else
            {
                UnionType *u = dynamic_cast<UnionType *>(t);
                assert(u);
                if (u)
                {
                    for (StructMember *m : u->getUnionMembers().getMembers())
                    {
                        if (m->findAnnotation(NO_SHARED_ANNOTATION))
                        {
                            templateData["sharedType"] = "union";
                            if (setDiscriminatorTemp(u, structType, structMember, isFunctionParam, templateData))
                            {
                                needTempVariable = true;
                            }

                            break;
                        }
                    }
                }
            }
        }
        return templateData;
    }

    //Check if member is byRef type. Add "*" for type and allocate space for data on server side.
    if (structMember && structMember->isByref() && !isFunctionParam && (t->isStruct() || t->isUnion() || t->isScalar() || t->isEnum()))
    {
        templateData["freeingCall2"] = m_templateData["freeData"];
        templateData["memberAllocation"] = allocateCall(false, name, t);
        localName = "*" + localName;
    }
    else
    {
        templateData["freeingCall2"] = make_template("", &params);
    }

    templateData["name"] = localName;

    if (t->isScalar() || t->isEnum())
    {
        templateData["pointerScalarTypes"] = false;
        if (!inDataContainer && structMember && structMember->getDirection() != kInDirection)
        {
            DataType *trueDataType = t->getTrueDataType();
            if (trueDataType->isScalar() || trueDataType->isEnum())
            {
                templateData["pointerScalarTypes"] = true;
            }
        }
    }

    switch (t->getDataType())
    {
        case DataType::kAliasType:
        {
            AliasType *aliasType = dynamic_cast<AliasType *>(t);
            assert(aliasType);
            return getEncodeDecodeCall(name, group, aliasType->getElementType(), structType, inDataContainer, structMember,
                                       needTempVariable, isFunctionParam);
        }
        case DataType::kArrayType:
        {
            static uint32_t arrayCounter;
            ArrayType *arrayType = dynamic_cast<ArrayType *>(t);
            assert(arrayType);
            DataType *elementType = arrayType->getElementType();
            DataType *trueElementType = elementType->getTrueDataType();

            std::string arrayName = name;
            ++arrayCounter;
            templateData["decode"] = m_templateData["decodeArrayType"];
            templateData["encode"] = m_templateData["encodeArrayType"];

            // To improve code serialization/deserialization for scalar types.
            templateData["builtinTypeName"] = ((m_def->getCodecType() != InterfaceDefinition::kBasicCodec) || trueElementType->isBool()) ? "" : getScalarTypename(elementType);

            giveBracesToArrays(arrayName);
            templateData["protoNext"] =
                getEncodeDecodeCall(format_string("%s[arrayCount%d]", arrayName.c_str(), arrayCounter),
                                    group, elementType, structType, true, structMember, needTempVariable, isFunctionParam);
            templateData["forLoopCount"] = format_string("arrayCount%d", arrayCounter);
            templateData["size"] = format_string("%d", arrayType->getElementCount());
            templateData["sizeTemp"] = format_string("%d", arrayType->getElementCount());
            templateData["isElementArrayType"] = trueElementType->isArray();
            if (generateServerFreeFunctions(structMember) && isNeedCallFree(t))
            {
                templateData["freeingCall"] = m_templateData["freeArray"];
            }
            templateData["pointerScalarTypes"] = false; // List is using array codec functions
            --arrayCounter;
            break;
        }
        case DataType::kBuiltinType:
        {
            getEncodeDecodeBuiltin(group, (BuiltinType *)t, templateData, structType, structMember, isFunctionParam);
            break;
        }
        case DataType::kEnumType:
        {
            needTempVariable = true;
            templateData["decode"] = m_templateData["decodeEnumType"];
            templateData["encode"] = m_templateData["encodeEnumType"];
            std::string typeName = t->getOutputName();
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
        case DataType::kFunctionType:
        {
            FunctionType *funType = dynamic_cast<FunctionType *>(t);
            assert(funType);
            const FunctionType::c_function_list_t &callbacks = funType->getCallbackFuns();
            if (callbacks.size() > 1)
            {
                templateData["tableName"] = "_" + funType->getName();
                templateData["tableSize"] = callbacks.size();
                needTempVariable = true;
            }
            else if (callbacks.size() == 1)
            {
                templateData["tableName"] = "";
                templateData["callbackName"] = callbacks[0]->getName();
            }
            else
            {
                throw semantic_error(format_string("line %d: Function has function type parameter (callback parameter), but in IDL is missing function definition, which can be passed there.", structMember->getFirstLine()).c_str());
            }
            templateData["encode"] = m_templateData["encodeFunctionType"];
            templateData["decode"] = m_templateData["decodeFunctionType"];
            break;
        }
        case DataType::kListType:
        {
            ListType *listType = dynamic_cast<ListType *>(t);
            assert(listType);
            DataType *elementType = listType->getElementType();
            DataType *trueElementType = elementType->getTrueDataType();

            bool isInOut = ((structMember && structMember->getDirection() == kInoutDirection) ||
                            (!isFunctionParam && group->getSymbolDirections(structType).count(kInoutDirection)));

            bool isTopDataType = (isFunctionParam && structMember && structMember->getDataType()->getTrueDataType() == t);
            templateData["useMallocOnClientSide"] = (!isInOut && !isTopDataType); // Because cpptempl don't know do
                                                                                  // correct complicated conditions like
                                                                                  // if(a || (b && c))

            templateData["mallocSizeType"] = getTypenameName(elementType, "");
            templateData["mallocType"] = getTypenameName(elementType, "*");
            templateData["needFreeingCall"] = (generateServerFreeFunctions(structMember) && isNeedCallFree(elementType));

            // To improve code serialization/deserialization for scalar types.
            templateData["builtinTypeName"] = ((m_def->getCodecType() != InterfaceDefinition::kBasicCodec) || trueElementType->isBool()) ? "" : getScalarTypename(elementType);

            if (generateServerFreeFunctions(structMember))
            {
                templateData["freeingCall"] = m_templateData["freeList"];
            }
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
            templateData["pointerScalarTypes"] = false;
            templateData["constantVariable"] = false;

            if (listType->hasLengthVariable())
            {
                templateData["hasLengthVariable"] = true;

                static uint32_t listCounter;
                ++listCounter;
                nextName = format_string("%s[listCount%d]", name.c_str(), listCounter);
                // needDealloc(templateData, t, nullptr, structMember);
                // length is global constant. Should be defined in IDL as array[global_constant] @nullable?
                Symbol *symbol = m_globals->getSymbol(listType->getLengthVariableName());
                if (symbol)
                {
                    ConstType *constType = dynamic_cast<ConstType *>(symbol);
                    assert(constType);
                    size = constType->getValue()->toString();
                    templateData["constantVariable"] = true;
                }
                else
                {
                    symbol = structType->getScope().getSymbol(listType->getLengthVariableName(), false);
                    if (!symbol)
                    {
                        //it is just number.
                        templateData["constantVariable"] = true;
                    }

                    // on client has to be used dereferencing out length variable when writeBinary/list is used.
                    if (isFunctionParam)
                    {
                        if (symbol)
                        {
                            StructMember *lengthVariable = dynamic_cast<StructMember *>(symbol);
                            assert(lengthVariable);
                            if (lengthVariable->getDirection() != kInDirection)
                            {
                                templateData["pointerScalarTypes"] = true;
                            }
                        }
                    }
                    size += listType->getLengthVariableName();
                }
                templateData["sizeTemp"] = std::string("lengthTemp_") + std::to_string(listCounter);
                templateData["dataTemp"] = std::string("dataTemp_") + std::to_string(listCounter);
                std::string maxSize = structMember->getAnnStringValue(MAX_LENGTH_ANNOTATION);
                if (maxSize != "")
                {
                    // preppend "data->" when maxSize is struct member
                    if (!isFunctionParam && structType)
                    {
                        Symbol *symbolMax = structType->getScope().getSymbol(maxSize, false);
                        if (symbolMax)
                        {
                            maxSize = "data->" + maxSize;
                        }
                    }
                    templateData["maxSize"] = maxSize;
                }
                else
                {
                    templateData["maxSize"] = templateData["sizeTemp"]->getvalue();
                }
                templateData["forLoopCount"] = std::string("listCount") + std::to_string(listCounter);
                templateData["isElementArrayType"] = trueElementType->isArray();
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
                    templateData["dataTemp"] = usedName + "_local";
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
            templateData["useBinaryCoder"] = isBinaryList(listType);
            templateData["protoNext"] =
                getEncodeDecodeCall(nextName, group, elementType, structType, true, structMember, needTempVariable, isFunctionParam);
            break;
        }
        case DataType::kStructType:
        {
            // needDealloc(templateData, t, structType, structMember);
            std::string typeName = t->getOutputName();
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

            if (generateServerFreeFunctions(structMember) && isNeedCallFree(t))
            {
                templateData["freeingCall"] = m_templateData["freeStruct"];
            }
            break;
        }
        case DataType::kUnionType:
        {
            UnionType *unionType = dynamic_cast<UnionType *>(t);

            // need casting discriminator variable?
            // set discriminator name
            if (setDiscriminatorTemp(unionType, structType, structMember, isFunctionParam, templateData))
            {
                needTempVariable = true;
            }

            /* NonEncapsulated unions as a function/structure param/member. */
            if (isFunctionParam || (structType && unionType->isNonEncapsulatedUnion()))
            {
                templateData["inDataContainer"] = inDataContainer;
                templateData["typeName"] = t->getOutputName();
                templateData["decode"] = m_templateData["decodeUnionParamType"];
                templateData["encode"] = m_templateData["encodeUnionParamType"];
                if (generateServerFreeFunctions(structMember) && isNeedCallFree(t))
                {
                    templateData["freeingCall"] = m_templateData["freeUnionType"];
                }

                // inout/out discriminator
                templateData["discrimPtr"] = false;
                if (isFunctionParam)
                {
                    if (structMember->getDirection() != kInDirection)
                    {
                        templateData["discrimPtr"] = true;
                    }
                }
            }
            else
            {
                /* Serialize/deserialize encapsulated and non-encapsulated unions is almost same. */
                assert(unionType);

                std::string disriminatorSeparator = "";

                /* Get disriminator separator for encapsulated unions. data->variable_name.case */
                if (!unionType->isNonEncapsulatedUnion())
                {
                    disriminatorSeparator = ".";
                }

                data_list unionCases;
                data_list unionCasesToFree;
                // call free function for this union, default not call any free function
                templateData["freeingCall"] = make_template("", &params);
                for (auto unionCase : unionType->getCases())
                {
                    data_map caseData;
                    caseData["name"] = unionCase->getCaseName();
                    caseData["value"] = unionCase->getCaseValue();
                    // if current case need call free function, default false
                    caseData["needCaseFreeingCall"] = false;
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
                        coderCalls["memberAllocation"] = "";
                        memberData["coderCall"] = coderCalls;

                        caseMembers.push_back(memberData);
                    }
                    else
                    {
                        bool casesNeedTempVariable = false;
                        // For each case member declaration, get its encode and decode data
                        for (auto caseMemberName : unionCase->getMemberDeclarationNames())
                        {
                            StructMember *memberDeclaration = unionCase->getUnionMemberDeclaration(caseMemberName);
                            std::string unionCaseName = name + disriminatorSeparator + memberDeclaration->getOutputName();
                            memberData["coderCall"] = getEncodeDecodeCall(unionCaseName, group, memberDeclaration->getDataType(),
                                                                          structType, true, structMember, casesNeedTempVariable, isFunctionParam);
                            if (generateServerFreeFunctions(structMember) &&
                                isNeedCallFree(memberDeclaration->getDataType()))
                            {
                                // set freeing function for current union
                                templateData["freeingCall"] = m_templateData["freeUnion"];
                                // current case need free memory
                                caseData["needCaseFreeingCall"] = true;
                                // current member need free memory
                                memberData["isNeedFreeingCall"] = true;
                            }
                            else
                            {
                                // current member don't need free memory
                                memberData["isNeedFreeingCall"] = false;
                            }
                            caseMembers.push_back(memberData);
                            if (casesNeedTempVariable)
                            {
                                needTempVariable = true;
                            }
                        }
                    }
                    caseData["members"] = caseMembers;
                    unionCases.push_back(caseData);
                }
                templateData["cases"] = unionCases;
                templateData["encode"] = m_templateData["encodeUnionType"];
                templateData["decode"] = m_templateData["decodeUnionType"];
            }
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
    if (trueDataType->isStruct() || trueDataType->isArray() || trueDataType->isUnion())
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
        if (!trueDataType->isBuiltin() && !trueDataType->isEnum() && !trueDataType->isList() && !trueDataType->isArray())
        {
            result = "*";
        }
        if (structMember->findAnnotation(SHARED_ANNOTATION))
        {
            result += "*";
        }
    }
    else if (structMemberDir == kInoutDirection)
    {
        if (!trueDataType->isBuiltin() && !trueDataType->isEnum() && !trueDataType->isList() && !trueDataType->isArray())
        {
            result = "*";
        }
        if (structMember->findAnnotation(SHARED_ANNOTATION))
        {
            result += "*";
        }
    }
    else
    {
        if ((trueDataType->isStruct() || trueDataType->isUnion()) || (trueDataType->isScalar() && structMember->findAnnotation(SHARED_ANNOTATION)))
        {
            result = "*";
        }
    }

    return result;
}

std::string CGenerator::firstAllocOnReturnWhenIsNeed(std::string name, DataType *dataType)
{
    DataType *trueDataType = dataType->getTrueDataType();
    if ((trueDataType->isArray() || trueDataType->isStruct() || trueDataType->isUnion()) && !dataType->findAnnotation(SHARED_ANNOTATION))
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
    if (!structMember->findAnnotation(SHARED_ANNOTATION))
    {
        if (structMemberDir == kInoutDirection)
        {
            if (!trueDataType->isBuiltin() && !trueDataType->isEnum() && !trueDataType->isList() && !trueDataType->isArray())
            {
                return allocateCall(false, name, structMember);
            }
        }
        else if (structMemberDir == kInDirection)
        {
            if (trueDataType->isStruct() || trueDataType->isUnion())
            {
                return allocateCall(false, name, structMember);
            }
        }
        else if (structMember->getDirection() == kOutDirection)
        {
            if (!trueDataType->isBuiltin() && !trueDataType->isEnum() && !trueDataType->isArray())
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
                std::string returnValue = allocateCall(false, name, structMember);
                if (m_templateData["generateAllocErrorChecks"]->getvalue() == "true")
                {
                    returnValue += format_string("\nelse\n{\n    %s[0]=\'\\0\';\n}", name.c_str());
                }
                else
                {
                    returnValue += format_string("\n%s[0]=\'\\0\';", name.c_str());
                }
                return returnValue;
            }
        }
    }
    return "";
}

bool CGenerator::isNeedCallFree(DataType *dataType)
{
    DataType *trueDataType = dataType->getTrueDataType();
    switch (trueDataType->getDataType())
    {
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
        case DataType::kStructType:
        {
            StructType *structType = dynamic_cast<StructType *>(trueDataType);
            assert(structType);
            set<DataType *> loopDetection;
            return structType->containListMember() || structType->containStringMember() || containsByrefParamToFree(structType, loopDetection);
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

void CGenerator::setCallingFreeFunctions(Symbol *symbol, cpptempl::data_map &info, bool returnType)
{
    StructMember *structMember = dynamic_cast<StructMember *>(symbol);
    assert(structMember);
    DataType *trueDataType = structMember->getDataType()->getTrueDataType();
    data_map firstFreeingCall1;
    firstFreeingCall1["firstFreeingCall"] = "";
    firstFreeingCall1["freeName"] = "";
    data_map firstFreeingCall2;
    firstFreeingCall2["firstFreeingCall"] = "";
    firstFreeingCall2["freeName"] = "";
    // When true then function parameter, else return type
    if (!symbol->findAnnotation(SHARED_ANNOTATION) && generateServerFreeFunctions(structMember))
    {
        if (!returnType)
        {
            switch (trueDataType->getDataType())
            {
                case DataType::kBuiltinType:
                case DataType::kStructType:
                case DataType::kUnionType:
                {
                    BuiltinType *builtinType = dynamic_cast<BuiltinType *>(trueDataType);
                    if (!builtinType || builtinType->isString())
                    {
                        _param_direction structMemberDir = structMember->getDirection();
                        std::string name = structMember->getOutputName();
                        if (!((trueDataType->isString()) && structMemberDir != kOutDirectionByref))
                        {
                            firstFreeingCall1["firstFreeingCall"] = m_templateData["freeData"];
                            firstFreeingCall1["freeName"] = name;
                        }
                        if (!trueDataType->isString() && structMemberDir == kOutDirectionByref)
                        {
                            name = "*" + name;
                            firstFreeingCall2["firstFreeingCall"] = m_templateData["freeData"];
                            firstFreeingCall2["freeName"] = name;
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }
        else
        {
            if (trueDataType->isArray() || trueDataType->isStruct() || trueDataType->isUnion())
            {
                firstFreeingCall1["firstFreeingCall"] = m_templateData["freeData"];
                firstFreeingCall1["freeName"] = "result";
            }
        }
    }
    info["firstFreeingCall1"] = firstFreeingCall1;
    info["firstFreeingCall2"] = firstFreeingCall2;
}

std::string CGenerator::allocateCall(bool pointer, const std::string &name, Symbol *symbol)
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
        assert(dataType);
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
        size = structMember->getAnnStringValue(MAX_LENGTH_ANNOTATION);
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
    if (m_templateData["generateAllocErrorChecks"]->getvalue() == "true")
    {
        returnVal += format_string("\nif (%s == NULL)\n{\n    err = kErpcStatus_MemoryError;\n}", name.c_str());
    }
    return returnVal;
}

std::string CGenerator::returnSpaceWhenNotEmpty(const std::string &param)
{
    return (param == "") ? "" : " ";
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

bool CGenerator::containsByrefParamToFree(DataType *dataType, set<DataType *> &dataTypes)
{
    // For loops from forward declaration detection.
    if (dataTypes.insert(dataType).second)
    {
        DataType *trueDataType = dataType->getTrueContainerDataType();
        if (trueDataType->isStruct())
        {
            StructType *structType = dynamic_cast<StructType *>(trueDataType);
            assert(structType != nullptr);

            for (StructMember *structMember : structType->getMembers())
            {
                if ((structMember->isByref() && !structMember->findAnnotation(SHARED_ANNOTATION)) || containsByrefParamToFree(structMember->getDataType(), dataTypes))
                {
                    return true;
                }
            }
        }
        else if (trueDataType->isUnion())
        {
            UnionType *unionType = dynamic_cast<UnionType *>(trueDataType);
            assert(unionType != nullptr);

            for (StructMember *structMember : unionType->getUnionMembers().getMembers())
            {
                if ((structMember->isByref() && !structMember->findAnnotation(SHARED_ANNOTATION)) || containsByrefParamToFree(structMember->getDataType(), dataTypes))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

bool CGenerator::isListStruct(StructType *structType)
{
    // if structure is transformed list<> to struct{list<>}
    for (StructType *structList : m_structListTypes)
    {
        if (structType == structList)
        {
            return true;
        }
    }
    return false;
}

bool CGenerator::isBinaryStruct(StructType *structType)
{
    // if structure contains one member list<>
    if (structType->getMembers().size() == 1 && structType->getMembers()[0]->getDataType()->isList())
    {
        /*
         * If list is same as in s_listBinaryTypes.
         * This list is always in 0-position of vector s_listBinaryTypes.
         */
        if (m_listBinaryTypes.size() && structType->getMembers()[0]->getDataType() == m_listBinaryTypes.at(0))
        {
            return true;
        }
    }
    return false;
}

bool CGenerator::isBinaryList(ListType *listType)
{

    // If list is same as in s_listBinaryTypes.
    for (ListType *list : m_listBinaryTypes)
    {
        if (listType == list)
        {
            return true;
        }
    }
    return false;
}

bool CGenerator::generateServerFreeFunctions(StructMember *structMember)
{
    return (structMember == nullptr || structMember->findAnnotation(RETAIN_ANNOTATION) == nullptr);
}

void CGenerator::setNoSharedAnn(Symbol *parentSymbol, Symbol *childSymbol)
{
    if (Annotation *ann = childSymbol->findAnnotation(NO_SHARED_ANNOTATION))
    {
        parentSymbol->addAnnotation(*ann);
    }
}

bool CGenerator::setDiscriminatorTemp(UnionType *unionType, StructType *structType, StructMember *structMember, bool isFunctionParam, cpptempl::data_map &templateData)
{
    bool needTempVariable = false;
    if (structType)
    {
        std::string discriminatorName;
        Symbol *disSymbol;

        if (unionType->isNonEncapsulatedUnion())
        {
            discriminatorName = structMember->getAnnStringValue(DISCRIMINATOR_ANNOTATION);
            disSymbol = m_globals->getSymbol(discriminatorName, false);
            if (isFunctionParam || disSymbol)
            {
                templateData["dataLiteral"] = "";
            }
            else
            {
                templateData["dataLiteral"] = "data->";
            }
        }
        else
        {
            disSymbol = nullptr;
            templateData["dataLiteral"] = "data->";
            discriminatorName = unionType->getDiscriminatorName();
        }

        DataType *disType;
        if (disSymbol)
        {
            templateData["isConstant"] = true;
            ConstType *constType = dynamic_cast<ConstType *>(disSymbol);
            assert(constType);
            disType = constType->getDataType();
        }
        else
        {
            templateData["isConstant"] = false;
            disSymbol = structType->getScope().getSymbol(discriminatorName);
            assert(disSymbol);
            StructMember *disMember = dynamic_cast<StructMember *>(disSymbol);
            assert(disMember);
            disType = disMember->getDataType();
        }

        BuiltinType *disBuiltin = dynamic_cast<BuiltinType *>(disType->getTrueDataType());
        if (disBuiltin && disBuiltin->getBuiltinType() == BuiltinType::kInt32Type)
        {
            templateData["castDiscriminator"] = false;
        }
        else
        {
            needTempVariable = true;
            templateData["castDiscriminator"] = true;
            templateData["discriminatorType"] = disType->getName();
        }
        templateData["discriminatorName"] = discriminatorName;
    }
    else
    {
        templateData["discriminatorName"] = "discriminator";
        templateData["dataLiteral"] = "";
        templateData["castDiscriminator"] = false;
    }
    return needTempVariable;
}

std::string CGenerator::getScalarTypename(DataType *dataType)
{
    if (dataType->getTrueDataType()->isScalar())
    {
        if (dataType->isAlias())
        {
            return dataType->getName();
        }
        else
        {
            BuiltinType *builtinType = dynamic_cast<BuiltinType *>(dataType);
            assert(builtinType);
            return getBuiltinTypename(builtinType);
        }
    }
    else
    {
        return "";
    }
}

std::string CGenerator::getDirection(_param_direction direction)
{
    switch (direction)
    {
        case kInDirection:
            return "kInDirection";
        case kOutDirection:
            return "kOutDirection";
        case kInoutDirection:
            return "kInoutDirection";
        case kReturn:
            return "kReturn";
        default:
            throw semantic_error("Unsupported direction type");
    }
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
