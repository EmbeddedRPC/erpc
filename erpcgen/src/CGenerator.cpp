/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
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

//ToDo: Temporary solution.
std::vector<UnionType *> unionsToClient;
std::vector<UnionType *> unionsToServer;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////
CGenerator::CGenerator(InterfaceDefinition *def, uint16_t idlCrc16)
: Generator(def, idlCrc16)
, m_generateClientOutput(true)
, m_generateServerOutput(true)
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
}

void CGenerator::generateOutputFiles(const std::string &fileNameExtension)
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
    // generate common types header file
    m_templateData["commonTypesFile"] = "";
    if (m_def->hasProgramSymbol())
    {
        Annotation *anno = m_def->getProgramSymbol()->findAnnotation(TYPES_HEADER_ANNOTATION);
        if (nullptr != anno)
        {
            // get name of types header file from annotation
            std::string commonTypesHeaderFileName = anno->getValueObject()->toString();

            m_templateData["commonGuardMacro"] = generateIncludeGuardName(commonTypesHeaderFileName);
            m_templateData["genCommonTypesFile"] = true;
            generateOutputFile(commonTypesHeaderFileName, "c_common_header", m_templateData, kCCommonHeader);

            // set name of types header file, which is used in common header file
            m_templateData["commonTypesFile"] = commonTypesHeaderFileName;
        }
    }

    // generate common header file
    fileName += ".h";
    m_templateData["commonGuardMacro"] = generateIncludeGuardName(fileName);
    m_templateData["commonHeaderName"] = fileName;
    m_templateData["genCommonTypesFile"] = false;
    generateOutputFile(fileName, "c_common_header", m_templateData, kCCommonHeader);
}

void CGenerator::generateClientSourceFile(std::string fileName)
{
    m_templateData["source"] = "client";
    fileName += "_client.cpp";
    m_templateData["clientSourceName"] = fileName;
    //TODO: temporary workaround for tests
    if (fileName.compare("test_unit_test_common_client.cpp") == 0)
    {
        m_templateData["functions"] = "";
    }
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
    if (fileName.compare("test_unit_test_common_client.cpp") == 0)
    {
        m_templateData["functions"] = "";
    }
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

    m_globals->replaceSymbol(replacedBuiltinType, newStruct);
    m_listBinaryTypes.insert(m_listBinaryTypes.begin(), listType);
}

DataType *CGenerator::transformDataType(DataType *dataType, _param_direction direction)
{
    switch (dataType->getDataType())
    {
        case DataType::kAliasType:
        {
            AliasType *aliasType = dynamic_cast<AliasType *>(dataType);
            assert(aliasType);
            aliasType->setElementType(transformDataType(aliasType->getElementType(), direction));
            break;
        }
        case DataType::kArrayType:
        {
            ArrayType *arrayType = dynamic_cast<ArrayType *>(dataType);
            assert(arrayType);
            arrayType->setElementType(transformDataType(arrayType->getElementType(), direction));
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
            DataType *elementType = transformDataType(listType->getElementType(), direction);
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
                                if (direction != kNone)
                                {
                                    structType->addStructDirectionType(direction);
                                }

                                return symDataType;
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
                if (direction != kNone)
                {
                    newStruct->addStructDirectionType(direction);
                }

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

                return newStruct;
            }
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
            if (isListStruct(structType))
            {
                /* Not necessary catch return value. Should be same (already transformed). This is only because of
                 adding direction type. */
                transformDataType(structMember->getDataType()->getTrueContainerDataType(), direction);
                break;
            }

            if (isBinaryStruct(structType))
            {
                break;
            }

            for (StructMember *structMember : structType->getMembers())
            {
                setBinaryList(structMember);
                setNoSharedAnn(structType, structMember);
                structMember->setDataType(transformDataType(structMember->getDataType(), direction));
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

            //ToDo: Temporary solution.
            if (direction == kInDirection || direction == kInoutDirection)
            {
                unionsToServer.push_back(currentUnion);
            }
            if (direction == kOutDirection || direction == kInoutDirection || direction == kReturn)
            {
                unionsToClient.push_back(currentUnion);
            }

            for (auto &unionMember : currentUnion->getUnionMemberDeclarations())
            {
                setBinaryList(&unionMember);
                setNoSharedAnn(currentUnion, &unionMember);
                unionMember.setDataType(transformDataType(unionMember.getDataType(), direction));
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

void CGenerator::transformDataTypes()
{
    for (auto it : m_globals->getSymbolsOfType(DataType::kStructTypeSymbol))
    {
        StructType *structType = dynamic_cast<StructType *>(it);
        assert(structType);
        transformDataType(structType, kNone);
    }

    for (auto it : m_globals->getSymbolsOfType(DataType::kAliasTypeSymbol))
    {
        AliasType *aliasType = dynamic_cast<AliasType *>(it);
        assert(aliasType);
        transformDataType(aliasType, kNone);
    }
    for (auto it : m_globals->getSymbolsOfType(Symbol::kInterfaceSymbol))
    {
        Interface *iface = dynamic_cast<Interface *>(it);
        assert(iface);
        for (Function *fn : iface->getFunctions())
        {
            _param_direction pramDirection = (fn->getReturnType()->findAnnotation(SHARED_ANNOTATION)) ? kNone : kReturn;
            fn->setReturnType(transformDataType(fn->getReturnType(), pramDirection));
            auto params = fn->getParameters().getMembers();
            for (auto mit : params)
            {
                pramDirection = (mit->findAnnotation(SHARED_ANNOTATION)) ? kNone : mit->getDirection();
                setBinaryList(mit);
                mit->setDataType(transformDataType(mit->getDataType(), pramDirection));
            }
        }
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

    /* Need handle nested calls. */
    m_templateData["nestedCall"] = false;

    data_list empty;
    m_templateData["enums"] = empty;
    m_templateData["aliases"] = empty;
    m_templateData["structs"] = empty;
    m_templateData["unions"] = empty;
    m_templateData["consts"] = empty;
    m_templateData["functions"] = empty;

    // Keil need extra pragma option when unions are used.
    m_templateData["usedUnionType"] = false;

    parseSubtemplates();

    transformDataTypes();

    makeIncludesTemplateData(m_templateData);

    interfaceLists_t interfaceLists = makeInterfacesTemplateData();

    makeConstTemplateData();

    makeEnumsTemplateData();

    makeAliasesTemplateData();

    makeUnionsStructsTemplateData();

    generateCrcFile();

    generateInterfaceOutputFiles(m_templateData, interfaceLists);
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
    Log::info("Functions:\n");
    data_list functions;
    for (Symbol *functionTypeSymbol : m_globals->getSymbolsOfType(Symbol::kFunctionTypeSymbol))
    {
        FunctionType *functionType = dynamic_cast<FunctionType *>(functionTypeSymbol);
        data_map functionInfo;

        // aware of external function definitions
        if (!functionType->findAnnotation(EXTERNAL_ANNOTATION))
        {
            AliasType *a = new AliasType(getFunctionPrototype(functionType), functionType);
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
        Annotation *externAnnotation = aliasType->findAnnotation(EXTERNAL_ANNOTATION);
        if (!externAnnotation)
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

void CGenerator::makeUnionsStructsTemplateData()
{
    makeUnionsTemplateData();
    makeStructsTemplateData();

    //mix structures and unions definitions
    data_list &unions = dynamic_cast<DataList *>(m_templateData["unions"].get().get())->getlist();
    data_list &structs = dynamic_cast<DataList *>(m_templateData["structs"].get().get())->getlist();

    if (unions.size() != 0 || structs.size() != 0)
    {
        if (unions.size() == 0)
        {
            m_templateData["unionsStructs"] = structs;
        }
        else if (structs.size() == 0)
        {
            m_templateData["unionsStructs"] = unions;
        }
        else
        {
            data_list unionsStructs;
            int structIt = 0;
            int unionIt = 0;

            SymbolScope::symbol_vector_t unionTypeSymbolVector = m_globals->getSymbolsOfType(Symbol::kUnionTypeSymbol);
            SymbolScope::symbol_vector_t structureTypeSymbolVector = m_globals->getSymbolsOfType(Symbol::kStructTypeSymbol);
            for (; structIt < structureTypeSymbolVector.size();)
            {
                StructType *structType = dynamic_cast<StructType *>(structureTypeSymbolVector.at(0));
                assert(structType);
                if (isListStruct(structType) || isBinaryStruct(structType) || unionIt >= unions.size() || structType->getFirstLine() < structureTypeSymbolVector.at(unionIt)->getFirstLine())
                {
                    unionsStructs.push_back(dynamic_cast<DataMap *>(structs.at(structIt).get().get())->getmap());
                    ++structIt;
                }
                else
                {
                    unionsStructs.push_back(dynamic_cast<DataMap *>(unions.at(unionIt).get().get())->getmap());
                    ++unionIt;
                }
            }

            //add rest of unions declared after structures
            for (; unionIt < unions.size(); ++unionIt)
            {
                unionsStructs.push_back(dynamic_cast<DataMap *>(unions.at(unionIt).get().get())->getmap());
            }
            m_templateData["unionsStructs"] = unionsStructs;
        }
    }
}

void CGenerator::makeStructsTemplateData()
{
    Log::info("Structs:\n");
    data_list structs;
    data_list structsToClient;
    data_list structsToServer;
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

        // Structure type definitions are mixed with unions.
        structInfo["isStruct"] = true;

        structInfo["noSharedMem"] = (structType->findAnnotation(NO_SHARED_ANNOTATION) != nullptr);

        setToCore(structType, structsToClient, structsToServer, structInfo);

        if (structType->containStringMember() || structType->containListMember())
        {
            structsServerFree.push_back(structInfo);
        }

        structs.push_back(structInfo);
    }
    m_templateData["structs"] = structs;
    m_templateData["structsToClient"] = structsToClient;
    m_templateData["structsToServer"] = structsToServer;
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
    structInfo["name"] = make_data(structType->getOutputName());
    structInfo["needTempVariable"] = false;
    setStructMembersTemplateData(structType, structInfo);
    setTemplateComments(structType, structInfo);
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
        data_map member_info;
        DataType *dataType = member->getDataType();
        DataType *trueDataType = member->getDataType()->getTrueDataType();
        std::string memberName = member->getOutputName();

        // Handle nullable annotation
        bool isNullable = ((member->findAnnotation(NULLABLE_ANNOTATION) != nullptr) && (trueDataType->isBinary() || trueDataType->isString() || trueDataType->isList() || trueDataType->isFunction() || (trueDataType->isStruct() && (isListStruct(dynamic_cast<StructType *>(trueDataType)) || (isBinaryStruct(dynamic_cast<StructType *>(trueDataType)))))));
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

        std::string name = "data->" + memberName;
        // Info for declaring struct in common header
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
        setTemplateComments(member, member_info);

        // Skip data serialization for variables placed as @length value for lists.
        // Skip data serialization for variables used as discriminator for unions.
        // These prevent to serialized data twice.
        StructMember *referencedFrom = findParamReferencedFrom(structType->getMembers(), member->getName());
        member_info["serializedViaMember"] = (referencedFrom) ? referencedFrom->getOutputName() : "";

        if (referencedFrom && !member->findAnnotation(SHARED_ANNOTATION))
        {
            Log::debug("Skipping EncodeDecode member %s with paramType %s (it's serialized with member %s).\n", memberName.c_str(), dataType->getName().c_str(), referencedFrom->getName().c_str());
        }
        else
        {
            bool needTempVariable = false;
            Log::debug("Calling EncodeDecode member %s with paramType %s.\n", memberName.c_str(), dataType->getName().c_str());

            // Subtemplate setup for read/write struct calls
            member_info["coderCall"] = getEncodeDecodeCall(name, dataType, structType, true, member, needTempVariable, false);

            if (needTempVariable)
            {
                structInfo["needTempVariable"] = true;
            }
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

void CGenerator::makeUnionsTemplateData()
{
    Log::info("Unions:\n");
    m_templateData["nonExternalUnion"] = false;
    data_list unions;
    data_list unionsListToClient;
    data_list unionsListToServer;
    data_list unionsServerFree;

    /* Get non-encapsulated unions declarations. */
    for (Symbol *unionTypeSymbol : m_globals->getSymbolsOfType(Symbol::kUnionTypeSymbol))
    {
        UnionType *unionType = dynamic_cast<UnionType *>(unionTypeSymbol);
        Log::info("%s\n", unionType->getDescription().c_str());
        data_map unionInfo;

        // Structure type definitions are mixed with unions.
        unionInfo["isStruct"] = false;

        // aware of external union definitions
        if (unionType->findAnnotation(EXTERNAL_ANNOTATION))
        {
            unionInfo["isExternal"] = true;
        }
        else
        {
            m_templateData["nonExternalUnion"] = true;
            unionInfo["isExternal"] = false;
        }

        unionInfo["name"] = unionType->getOutputName();
        unionInfo["membersData"] = getUnionMembersData(unionType, "");
        setTemplateComments(unionType, unionInfo);
        bool needTempVariable = false;
        unionInfo["coderCall"] = getEncodeDecodeCall("data->", unionType, nullptr, true, nullptr, needTempVariable, false);
        unionInfo["needTempVariable"] = needTempVariable;

        unionInfo["noSharedMem"] = (unionType->findAnnotation(NO_SHARED_ANNOTATION) != nullptr);

        //ToDo: Temporary solution.
        if (std::find(unionsToClient.begin(), unionsToClient.end(), unionType) != unionsToClient.end())
        {
            unionsListToClient.push_back(unionInfo);
        }
        if (std::find(unionsToServer.begin(), unionsToServer.end(), unionType) != unionsToServer.end())
        {
            unionsListToServer.push_back(unionInfo);
        }

        bool needUnionsServerFree = false; //free unions on server side.
        for (auto unionCase : unionType->getUniqueCases())
        {
            if (!unionCase->caseMemberIsVoid())
            {
                for (auto memberName : unionCase->getMemberDeclarationNames())
                {
                    StructMember *unionMember = unionCase->getUnionMemberDeclaration(memberName);
                    DataType *trueMemberDataType = unionMember->getDataType()->getTrueDataType();
                    if (trueMemberDataType->isList() || trueMemberDataType->isBinary() || trueMemberDataType->isString())
                    {
                        needUnionsServerFree = true;
                        break;
                    }
                    else if (trueMemberDataType->isStruct())
                    {
                        StructType *structType = dynamic_cast<StructType *>(trueMemberDataType);
                        assert(structType);
                        if (structType->containStringMember() || structType->containListMember())
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
        if (needUnionsServerFree)
        {
            unionsServerFree.push_back(unionInfo);
        }

        unions.push_back(unionInfo);
    }

    m_templateData["unions"] = unions;
    m_templateData["unionsToClient"] = unionsListToClient;
    m_templateData["unionsToServer"] = unionsListToServer;
    m_templateData["unionsServerFree"] = unionsServerFree;
}

/*
 * TODO: Change to use
 */
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

data_map CGenerator::getFunctionTemplateData(Function *fn, int fnIndex)
{
    data_map info;
    std::string proto = getFunctionPrototype(fn);
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
    info["needNullVariableOnClient"] = false;
    info["needNullVariableOnServer"] = false;

    /* Is function declared as external? */
    Annotation *externalAnn = fn->findAnnotation(EXTERNAL_ANNOTATION);
    if (!externalAnn)
    {
        info["isNonExternalFunction"] = true;
    }
    else
    {
        info["isNonExternalFunction"] = false;
    }

    if ((fn->findAnnotation(NESTED_CALL) != nullptr))
    {
        info["nestedCall"] = true;
        m_templateData["nestedCall"] = true;
    }
    else
    {
        info["nestedCall"] = false;
    }

    // Get return value info
    data_map returnInfo;
    returnInfo["type"] = getTypeInfo(fn->getReturnType(), true);
    DataType *dataType = fn->getReturnType();
    DataType *trueDataType = dataType->getTrueDataType();
    if (!trueDataType->isVoid())
    {
        std::string result = "result";
        bool needTempVariable = false;
        returnInfo["firstAlloc"] = firstAllocOnReturnWhenIsNeed(result, dataType);
        setCallingFreeFunctions(dataType, returnInfo);
        std::string extraPointer = getExtraPointerInReturn(dataType);
        std::string resultVariable = extraPointer + returnSpaceWhenNotEmpty(extraPointer) + result;
        Annotation *annShared = dataType->findAnnotation(SHARED_ANNOTATION);
        if (!annShared)
        {
            result = extraPointer + result;
        }
        returnInfo["coderCall"] = getEncodeDecodeCall(result, dataType, nullptr, false, nullptr, needTempVariable, true);
        returnInfo["shared"] = (annShared) ? true : false;
        resultVariable = getTypenameName(dataType, resultVariable);
        info["needTempVariableClient"] = needTempVariable;
        returnInfo["resultVariable"] = resultVariable;
        returnInfo["isNeedFreeingCall"] = (!annShared && isNeedCallFree(dataType));
        returnInfo["errorReturnValue"] = getErrorReturnValue(fn);
        returnInfo["isNullReturnType"] =
            ((!trueDataType->isBuiltin() && !trueDataType->isEnum()) || trueDataType->isString());
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

        // Handle nullable annotation.
        bool isNullable = (paramTrueType->isString() || (!paramTrueType->isBuiltin() && !paramTrueType->isArray() && !paramTrueType->isEnum()));
        paramInfo["isNullParam"] = isNullable;
        isNullable = ((param->findAnnotation(NULLABLE_ANNOTATION) != nullptr) && isNullable);
        paramInfo["isNullable"] = isNullable;
        // Set flags to indicate whether a local isNull variable is needed on the
        // server and client sides.
        //
        // If needNullVariableX is true, we don't want to overwrite it to false
        // if a later parameter is not nullable. So, we will only try to set it
        // if it is not true. Once the variable's value is true, we know we need
        // a null variable at least once.
        if (isNullable)
        {
            if (param->getDirection() == kInDirection || param->getDirection() == kInoutDirection)
            {
                info["needNullVariableOnServer"] = true;
            }
            if (param->getDirection() == kOutDirection || param->getDirection() == kInoutDirection)
            {
                info["needNullVariableOnClient"] = true;
            }
        }

        // Check if max_length annotation belongs to "in" param
        // TODO: Should be global, also for PythonGenerator
        if (paramTrueType->isList() || paramTrueType->isString())
        {
            Annotation *ann = param->findAnnotation(MAX_LENGTH_ANNOTATION);
            if (ann)
            {
                std::string maxLengthName = ann->getValueObject()->toString();
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
                                          ann->getLocation().m_firstLine, structMember->getLocation().m_firstLine));
                    }
                }
            }
        }

        Annotation *annShared = param->findAnnotation(SHARED_ANNOTATION);

        paramInfo["mallocServer"] = firstAllocOnServerWhenIsNeed(name, param);
        paramInfo["mallocServerOut"] = firtAllocOutParamOnServerWhenIsNeed(name, param);
        setCallingFreeFunctions(param, paramInfo);
        std::string encodeDecodeName;
        if (annShared)
        {
            encodeDecodeName = name;
            name = "*" + name;
        }
        else
        {
            encodeDecodeName = name = getExtraDirectionPointer(param) + name;
        }
        std::string variable = getTypenameName(paramType, name);
        paramInfo["variable"] = variable;
        paramInfo["name"] = name;
        paramInfo["type"] = getTypeInfo(paramType, false);

        // Skip data serialization for variables placed as @length value for lists.
        // Skip data serialization for variables used as discriminator for unions.
        // These prevent to serialized data twice.
        StructMember *referencedFrom = findParamReferencedFrom(fnParams, param->getName());
        paramInfo["serializedViaMember"] = (referencedFrom) ? referencedFrom->getOutputName() : "";

        if (referencedFrom && !annShared)
        {
            Log::debug("Skipping EncodeDecode param %s with paramType %s (it's serialized with param %s).\n", param->getName().c_str(), paramType->getName().c_str(), referencedFrom->getName().c_str());
            paramInfo["shared"] = false;
        }
        else
        {
            Log::debug("Calling EncodeDecode param %s with paramType %s.\n", param->getName().c_str(), paramType->getName().c_str());

            paramInfo["coderCall"] = getEncodeDecodeCall(encodeDecodeName, paramType, &fn->getParameters(), false, param, needTempVariable, true);

            paramInfo["shared"] = (annShared) ? true : false;
        }

        setToCore(param, paramsToClient, paramsToServer, paramInfo);
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
        bool l_generateServerFunctionParamFreeFunctions = (!annShared && generateServerFunctionParamFreeFunctions(param));
        bool isNeedFreeingCall = (l_generateServerFunctionParamFreeFunctions && isNeedCallFree(paramType));
        paramInfo["isNeedFreeingCall"] = isNeedFreeingCall;
        if (l_generateServerFunctionParamFreeFunctions &&
            (isNeedFreeingCall || paramInfo["firstFreeingCall1"]->getmap()["name"]->getvalue() != "" ||
             paramInfo["firstFreeingCall2"]->getmap()["name"]->getvalue() != ""))
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

void CGenerator::setToCore(Symbol *symbolType, data_list &toClient, data_list &toServer, data_map &dataMap)
{
    dataMap["allocateToServer"] = false;
    dataMap["allocateToClient"] = false;
    dataMap["allocateToClientAndServer"] = false;
    switch (getDirection(symbolType))
    {
        case 1: // in
        {
            dataMap["allocateToServer"] = true;
            toServer.push_back(dataMap);
            break;
        }
        case 2: // out
        {
            dataMap["allocateToClient"] = true;
            toClient.push_back(dataMap);
            break;
        }
        case 3: // inout
        {
            dataMap["allocateToClientAndServer"] = true;
            toServer.push_back(dataMap);
            toClient.push_back(dataMap);
            break;
        }
        case 5: // ToDo: shared pointer
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
    Annotation *ann = fn->findAnnotation(ERROR_RETURN_ANNOTATION);
    DataType *dataType = fn->getReturnType()->getTrueDataType();
    if (ann)
    {
        if (!ann->hasValue() || ann->getValueObject()->toString() == "")
        {
            throw semantic_error(format_string("Expected value for @%s annotation on line %d", ERROR_RETURN_ANNOTATION,
                                               ann->getLocation().m_firstLine));
        }
        else
        {
            Value *returnVal = ann->getValueObject();
            if (dataType->isString())
            {
                return "(char *) " + returnVal->toString();
            }
            else if (dataType->isBuiltin())
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
    }
    else
    {
        if (dataType->isBuiltin() && !dataType->isString())
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
            if ((it->getDirection() != kInDirection) && (((trueDataType->isBuiltin() || trueDataType->isEnum()) &&
                                                          !trueDataType->isString()) ||
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

std::string CGenerator::getFunctionPrototype(FunctionBase *fn)
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
                if (!it->findAnnotation(SHARED_ANNOTATION) && (structType->hasStructDirectionType(kInoutDirection) &&
                                                               (structType->hasStructDirectionType(kOutDirection) ||
                                                                structType->hasStructDirectionType(kOutDirectionByref) ||
                                                                structType->hasStructDirectionType(kReturn))))
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

void CGenerator::getEncodeDecodeBuiltin(BuiltinType *t,
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
        // templateData["withoutAlloc"] = false;
        templateData["withoutAlloc"] = ((structMember && structMember->getDirection() == kInoutDirection) ||
                                        (structType && structType->hasStructDirectionType(kInoutDirection))) ?
                                           true :
                                           false;
        if (!isFunctionParam)
        {
            templateData["stringAllocSize"] = structMember->getOutputName() + "_len";
            templateData["stringLocalName"] = structMember->getOutputName();
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
                templateData["stringLocalName"] = structMember->getOutputName();
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
                                         bool &needTempVariable,
                                         bool isFunctionParam)
{
    // prepare data for template
    data_map templateData;
    string_vector params = string_vector(1, "");
    templateData["freeingCall"] = make_template("", &params);
    templateData["inDataContainer"] = inDataContainer;
    templateData["isElementArrayType"] = false;

    int pos = name.rfind("*");
    if ((pos == 0 || pos == 1) && (t->getTrueDataType()->isStruct() || t->getTrueDataType()->isUnion()) && inDataContainer == false)
    {
        templateData["name"] = name.substr(1, name.length());
    }
    else
    {
        templateData["name"] = name;
    }

    /* Check if member should be serialized as shared. */
    if ((structMember && structMember->findAnnotation(SHARED_ANNOTATION)) || (structType && structType->findAnnotation(SHARED_ANNOTATION)) || t->findAnnotation(SHARED_ANNOTATION))
    {
        templateData["funcParam"] = (structMember) ? true : false;
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
                    for (StructMember m : u->getUnionMemberDeclarations())
                    {
                        if (m.findAnnotation(NO_SHARED_ANNOTATION))
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

    switch (t->getDataType())
    {
        case DataType::kAliasType:
        {
            AliasType *aliasType = dynamic_cast<AliasType *>(t);
            assert(aliasType);
            return getEncodeDecodeCall(name, aliasType->getElementType(), structType, inDataContainer, structMember,
                                       needTempVariable, isFunctionParam);
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
                                    arrayType->getElementType(), structType, true, structMember, needTempVariable, isFunctionParam);
            templateData["forLoopCount"] = format_string("arrayCount%d", arrayCounter);
            templateData["size"] = format_string("%d", arrayType->getElementCount());
            templateData["sizeTemp"] = format_string("%d", arrayType->getElementCount());
            templateData["isElementArrayType"] = arrayType->getElementType()->getTrueDataType()->isArray();
            templateData["freeingCall"] =
                (generateServerFunctionParamFreeFunctions(structMember) && isNeedCallFree(t)) ?
                    m_templateData["freeArray"] :
                    make_template("", &params);
            templateData["pointerScalarTypes"] = false; // List is using array codec functions
            --arrayCounter;
            break;
        }
        case DataType::kBuiltinType:
        {
            getEncodeDecodeBuiltin((BuiltinType *)t, templateData, structType, structMember, isFunctionParam);
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

            bool isInOut = ((structMember && structMember->getDirection() == kInoutDirection) ||
                            (!isFunctionParam && structType->hasStructDirectionType(kInoutDirection)));
            bool isTopDataType = (isFunctionParam && structMember && structMember->getDataType()->getTrueDataType() == t);
            templateData["useMallocOnClientSide"] = (!isInOut && !isTopDataType); // Because cpptempl don't know do
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
                std::string maxSize = getMaxLength(structMember);
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
                getEncodeDecodeCall(nextName, listType->getElementType(), structType, true, structMember, needTempVariable, isFunctionParam);
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
            templateData["freeingCall"] =
                (generateServerFunctionParamFreeFunctions(structMember) && isNeedCallFree(t)) ?
                    m_templateData["freeStruct"] :
                    make_template("", &params);
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
                templateData["name"] = structMember->getOutputName();
                templateData["inDataContainer"] = inDataContainer;
                templateData["typeName"] = t->getOutputName();
                templateData["decode"] = m_templateData["decodeUnionParamType"];
                templateData["encode"] = m_templateData["encodeUnionParamType"];
                templateData["freeingCall"] = (generateServerFunctionParamFreeFunctions(structMember) && isNeedCallFree(t)) ?
                                                  m_templateData["freeUnionType"] :
                                                  make_template("", &params);

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
                            memberData["coderCall"] = getEncodeDecodeCall(unionCaseName, memberDeclaration->getDataType(),
                                                                          structType, true, structMember, casesNeedTempVariable, isFunctionParam);
                            if (generateServerFunctionParamFreeFunctions(structMember) &&
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
    else
    {
        if (trueDataType->isStruct() || trueDataType->isUnion())
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
    }
    return "";
}

std::string CGenerator::firtAllocOutParamOnServerWhenIsNeed(std::string name, StructMember *structMember)
{
    if (structMember->getDirection() == kOutDirection && !structMember->findAnnotation(SHARED_ANNOTATION))
    {
        DataType *trueDataType = structMember->getDataType()->getTrueDataType();
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
            return structType->containListMember() || structType->containStringMember();
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
    if (!symbol->findAnnotation(SHARED_ANNOTATION))
    {
        if (structMember)
        {
            DataType *trueDataType = structMember->getDataType()->getTrueDataType();
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
                            firstFreeingCall1["name"] = name;
                        }
                        if (!trueDataType->isString() && structMemberDir == kOutDirectionByref)
                        {
                            name = "*" + name;
                            firstFreeingCall2["firstFreeingCall"] = m_templateData["freeData"];
                            firstFreeingCall2["name"] = name;
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
            DataType *dataType = dynamic_cast<DataType *>(symbol);
            assert(dataType);
            if (dataType->getTrueDataType()->isArray() || dataType->getTrueDataType()->isStruct() || dataType->getTrueDataType()->isUnion())
            {
                firstFreeingCall1["firstFreeingCall"] = m_templateData["freeData"];
                firstFreeingCall1["name"] = "result";
            }
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
        else if (!in && !out && !ret && !inOut)
        {
            // ToDo: shared pointer.
            return 5;
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

bool CGenerator::generateServerFunctionParamFreeFunctions(StructMember *structMember)
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
            discriminatorName = structMember->findAnnotation(DISCRIMINATOR_ANNOTATION)->getValueObject()->toString();
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

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
