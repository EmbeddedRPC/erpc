/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
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
using namespace std;

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
extern const char *const kCCrc;

// number which makes list temporary variables unique.
static uint8_t listCounter = 0;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////
CGenerator::CGenerator(InterfaceDefinition *def)
: Generator(def, kC)
{
    /* Set copyright rules. */
    if (m_def->hasProgramSymbol())
    {
        Program *program = m_def->getProgramSymbol();
        string copyright = program->getMlComment();
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

    initCReservedWords();
}

void CGenerator::generateOutputFiles(const string &fileName)
{
    generateClientSourceFile(fileName);

    generateServerHeaderFile(fileName);
    generateServerSourceFile(fileName);

    generateCommonHeaderFiles(fileName);
}

void CGenerator::generateTypesHeaderFile()
{
    string typesHeaderFileName = m_templateData["commonTypesFile"]->getvalue();

    m_templateData["commonGuardMacro"] = generateIncludeGuardName(typesHeaderFileName);
    m_templateData["genCommonTypesFile"] = true;
    m_templateData["commonTypesFile"] = "";

    generateOutputFile(typesHeaderFileName, "c_common_header", m_templateData, kCCommonHeader);
}

void CGenerator::generateCommonHeaderFiles(const string &fileName)
{
    m_templateData["commonGuardMacro"] = generateIncludeGuardName(fileName + ".h");
    m_templateData["genCommonTypesFile"] = false;

    generateOutputFile(fileName + ".h", "c_common_header", m_templateData, kCCommonHeader);
}

void CGenerator::generateClientSourceFile(string fileName)
{
    m_templateData["source"] = "client";
    fileName += "_client.cpp";
    m_templateData["clientSourceName"] = fileName;

    // TODO: temporary workaround for tests
    m_templateData["unitTest"] = (fileName.compare("test_unit_test_common_client.cpp") == 0 ? false : true);

    generateOutputFile(fileName, "c_client_source", m_templateData, kCClientSource);
}

void CGenerator::generateServerHeaderFile(string fileName)
{
    fileName += "_server.h";
    m_templateData["serverGuardMacro"] = generateIncludeGuardName(fileName);
    m_templateData["serverHeaderName"] = fileName;
    generateOutputFile(fileName, "c_server_header", m_templateData, kCServerHeader);
}

void CGenerator::generateServerSourceFile(string fileName)
{
    m_templateData["source"] = "server";
    fileName += "_server.cpp";
    m_templateData["serverSourceName"] = fileName;

    // TODO: temporary workaround for tests
    m_templateData["unitTest"] = (fileName.compare("test_unit_test_common_server.cpp") == 0 ? false : true);

    generateOutputFile(fileName, "c_server_source", m_templateData, kCServerSource);
}

void CGenerator::generateCrcFile()
{
    string filename = "erpc_crc16.h";
    m_templateData["crcGuardMacro"] = generateIncludeGuardName(filename);
    generateOutputFile(filename, "c_crc", m_templateData, kCCrc);
}

void CGenerator::parseSubtemplates()
{
    string templateName = "c_coders";
    try
    {
        parse(kCCoders, m_templateData);
        templateName = "c_common_functions";
        parse(kCCommonFunctions, m_templateData);
    }
    catch (TemplateException &e)
    {
        throw TemplateException(format_string("Template %s: %s", templateName.c_str(), e.what()));
    }
}

DataType *CGenerator::findChildDataType(set<DataType *> &dataTypes, DataType *dataType)
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
            set<DataType *> localDataTypes;
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
            DataType *trueContainerDataType = listType->getTrueContainerDataType();
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
                uint32_t nameCount = 1;

                string structName =
                    format_string("list_%s_%d_t", getOutputName(trueContainerDataType, false).c_str(), nameCount);
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
                    structName =
                        format_string("list_%s_%d_t", getOutputName(trueContainerDataType, false).c_str(), nameCount);
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
                if (trueContainerDataType->isStruct())
                {
                    symbolPos = m_globals->getSymbolPos(trueContainerDataType) + 1;
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

        set<DataType *> dataTypesNew;
        findChildDataType(dataTypesNew, aliasType);
    }
}

void CGenerator::setBinaryList(StructMember *structMember)
{
    DataType *dataType = structMember->getDataType();
    if (dataType->isBinary())
    {
        Annotation *listLength = findAnnotation(structMember, LENGTH_ANNOTATION);
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
    Program *program = nullptr;
    bool generateAllocErrorChecks = true;
    bool generateInfraErrorChecks = true;
    if (m_def->hasProgramSymbol())
    {
        program = m_def->getProgramSymbol();
        generateAllocErrorChecks = (findAnnotation(program, NO_ALLOC_ERRORS_ANNOTATION) == nullptr);
        generateInfraErrorChecks = (findAnnotation(program, NO_INFRA_ERRORS_ANNOTATION) == nullptr);
    }
    /* Generate file with shim code version. */
    m_templateData["versionGuardMacro"] =
        generateIncludeGuardName(format_string("erpc_generated_shim_code_crc_%d", m_idlCrc16).c_str());

    m_templateData["generateInfraErrorChecks"] = generateInfraErrorChecks;
    m_templateData["generateAllocErrorChecks"] = generateAllocErrorChecks;
    m_templateData["generateErrorChecks"] = generateInfraErrorChecks || generateAllocErrorChecks;

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
    if (m_def->hasProgramSymbol() && findAnnotation(program, CRC_ANNOTATION) != nullptr)
    {
        generateCrcFile();
    }

    // check if structure/function parameters annotations are valid.
    for (Symbol *symbol : m_globals->getSymbolsOfType(Symbol::kFunctionTypeSymbol))
    {
        FunctionType *functionType = dynamic_cast<FunctionType *>(symbol);
        assert(functionType);
        scanStructForAnnotations(&functionType->getParameters(), true);
    }

    for (Symbol *symbol : m_globals->getSymbolsOfType(Symbol::kStructTypeSymbol))
    {
        StructType *structType = dynamic_cast<StructType *>(symbol);
        assert(structType);
        scanStructForAnnotations(structType, false);
    }

    for (Symbol *symbol : m_globals->getSymbolsOfType(Symbol::kInterfaceSymbol))
    {
        Interface *interface = dynamic_cast<Interface *>(symbol);
        assert(interface);
        for (Function *function : interface->getFunctions())
        {
            scanStructForAnnotations(&function->getParameters(), true);
        }
    }

    // transform alias data types
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
        m_templateData["commonTypesFile"] = getAnnStringValue(program, TYPES_HEADER_ANNOTATION);
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
        groupTemplate["callbacks"] = makeGroupCallbacksTemplateData(group);
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
        if (!findAnnotation(constVar, EXTERNAL_ANNOTATION))
        {
            DataType *constVarType = dynamic_cast<DataType *>(constVar->getDataType());
            assert(constVarType);
            Value *constVarValue = constVar->getValue();

            if (nullptr == constVarValue)
            {
                throw semantic_error(
                    format_string("line %d: Const pointing to null Value object.", constVar->getLastLine()).c_str());
            }

            /* Use char[] for constants. */
            if (constVarType->getTrueDataType()->isString())
            {
                constInfo["typeAndName"] = format_string("char %s [%d]", getOutputName(constVar).c_str(),
                                                         constVarValue->toString().size() + 1);
            }
            else
            {
                constInfo["typeAndName"] = getTypenameName(constVarType, getOutputName(constVar));
            }
            constInfo["name"] = getOutputName(constVar);

            string value;
            if (constVarType->isEnum())
            {
                if (constVarValue->getType() != kIntegerValue)
                {
                    throw semantic_error(format_string("line %d: Const enum pointing to non-integer Value object.",
                                                       constVar->getLastLine())
                                             .c_str());
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
                    Log::warning(format_string("Enum value '%s' is not pointing to any '%s' variable \n",
                                               constVarValue->toString().c_str(), constVarType->getName().c_str())
                                     .c_str());
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
            Log::info("Name=%s\tType=%s\tValue=%s\n", constVar->getName().c_str(), constVarType->getName().c_str(),
                      constVar->getValue()->toString().c_str());
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
        if (!findAnnotation(enumType, EXTERNAL_ANNOTATION))
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
    enumInfo["name"] = getOutputName(enumType);
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
        string memberDeclaration = getOutputName(member);
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

    /* type definitions of structures */
    int i = 0;
    for (auto it : m_globals->getSymbolsOfType(DataType::kStructTypeSymbol))
    {
        StructType *structType = dynamic_cast<StructType *>(it);
        assert(structType);
        if (structType->getName().compare("") != 0 && !findAnnotation(structType, EXTERNAL_ANNOTATION))
        {
            AliasType *a = new AliasType(getOutputName(structType), structType);
            aliasTypeVector.insert(aliasTypeVector.begin() + i++, a);
        }
    }

    /* type definitions of non-encapsulated unions */
    for (auto it : m_globals->getSymbolsOfType(DataType::kUnionTypeSymbol))
    {
        UnionType *unionType = dynamic_cast<UnionType *>(it);
        assert(unionType);
        if (!findAnnotation(unionType, EXTERNAL_ANNOTATION))
        {
            AliasType *a = new AliasType(getOutputName(unionType), unionType);
            aliasTypeVector.insert(aliasTypeVector.begin() + i++, a);
        }
    }

    /* type definitions of functions and table of functions */
    data_list functions;
    for (Symbol *functionTypeSymbol : m_globals->getSymbolsOfType(Symbol::kFunctionTypeSymbol))
    {
        FunctionType *functionType = dynamic_cast<FunctionType *>(functionTypeSymbol);
        assert(functionType);
        data_map functionInfo;

        // aware of external function definitions
        if (!findAnnotation(functionType, EXTERNAL_ANNOTATION))
        {
            AliasType *a = new AliasType(getFunctionPrototype(nullptr, functionType), functionType);
            a->setMlComment(functionType->getMlComment());
            a->setIlComment(functionType->getIlComment());

            /* Function type definition need be inserted after all parameters types definitions. */
            DataType *callbackParamType = functionType->getReturnStructMemberType()->getDataType();
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
                    if (callbackParamType == aliasTypeVector[aliasTypesIt])
                    {
                        // Add aliases in IDL declaration order.
                        int nextIt = aliasTypesIt + 1;
                        while (nextIt < aliasTypeVector.size())
                        {
                            AliasType *nextAlias = dynamic_cast<AliasType *>(aliasTypeVector[nextIt]);
                            assert(nextAlias);
                            if (nextAlias->getElementType()->isFunction())
                            {
                                ++nextIt;
                            }
                            else
                            {
                                break;
                            }
                        }

                        aliasTypeVector.insert(aliasTypeVector.begin() + nextIt, a);
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
        if (!findAnnotation(aliasType, EXTERNAL_ANNOTATION))
        {
            Log::info("%d: ", n);
            data_map aliasInfo;
            DataType *elementDataType = aliasType->getElementType();
            setTemplateComments(aliasType, aliasInfo);

            if (elementDataType->getName() != "")
            {
                string realType;
                if (elementDataType->isFunction())
                {
                    realType = getOutputName(aliasType);
                    aliasInfo["name"] = elementDataType->getName();
                }
                else
                {
                    realType = getTypenameName(elementDataType, getOutputName(aliasType));
                    aliasInfo["name"] = getOutputName(aliasType);
                }

                Log::info("%s\n", realType.c_str());

                /* For case typedef struct/union */
                if (elementDataType->getName() == aliasType->getName() ||
                    getOutputName(elementDataType, false) == aliasType->getName())
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
                aliasInfo["unnamedName"] = getOutputName(aliasType);
                switch (elementDataType->getDataType())
                {
                    case DataType::kStructType:
                    {
                        StructType *structType = dynamic_cast<StructType *>(elementDataType);
                        assert(structType);
                        aliasInfo["unnamed"] = getStructDefinitionTemplateData(
                            nullptr, structType, getStructDeclarationTemplateData(structType));
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
    return (aliasType != nullptr) ? getOutputName(aliasType) : "";
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

data_map CGenerator::getSymbolTemplateByName(const string &name)
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
    set<string> names;

    data_list symbolsToClient;
    data_list symbolsToServer;
    data_list symbolsServerFree;

    Log::info("Group symbols:\n");

    for (Symbol *symbol : group->getSymbols())
    {
        data_map info;
        const set<_param_direction> dirs = group->getSymbolDirections(symbol);
        if (dirs.size())
        {
            switch (symbol->getSymbolType())
            {
                case DataType::kStructTypeSymbol:
                {
                    StructType *structType = dynamic_cast<StructType *>(symbol);
                    assert(structType);

                    Log::info("%s\n", structType->getDescription().c_str());

                    string name = (structType->getName() != "") ? getOutputName(structType) : getAliasName(structType);

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
                        if (structType->containStringMember() || structType->containListMember() ||
                            containsByrefParamToFree(structType, loopDetection))
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

                    string name = getOutputName(unionType);

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

data_list CGenerator::makeGroupCallbacksTemplateData(Group *group)
{
    data_list functionTypes;
    std::map<FunctionType *, int> functionTypeMap;

    // need go trough functions instead of group symbols.
    for (Interface *interface : group->getInterfaces())
    {
        for (Function *function : interface->getFunctions())
        {
            FunctionType *functionType = function->getFunctionType();
            if (functionType)
            {
                bool isPresent = (functionTypeMap.find(functionType) != functionTypeMap.end());
                if (isPresent)
                {
                    ++functionTypeMap[functionType];
                }
                else
                {
                    functionTypeMap[functionType] = 1;
                }
            }
        }
    }

    for (std::map<FunctionType *, int>::iterator it = functionTypeMap.begin(); it != functionTypeMap.end(); ++it)
    {
        if (it->second > 1)
        {
            functionTypes.push_back(getFunctionTypeTemplateData(group, it->first));
        }
    }

    return functionTypes;
}

data_map CGenerator::getStructDeclarationTemplateData(StructType *structType)
{
    data_map info;
    info["name"] = (structType->getName() != "" ? getOutputName(structType) : getAliasName(structType));
    info["type"] = "struct";

    setTemplateComments(structType, info);

    // external annotation
    if (findAnnotation(structType, EXTERNAL_ANNOTATION))
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

        string memberName = getOutputName(member);
        member_info["name"] = memberName;

        DataType *trueDataType = member->getDataType()->getTrueDataType();
        // Check if member is byRef type. Add "*" for type and allocate space for data on server side.
        if (member->isByref() &&
            (trueDataType->isStruct() || trueDataType->isUnion() || trueDataType->isScalar() || trueDataType->isEnum()))
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
    // reset list numbering.
    listCounter = 0;

    structInfo["hasNullableMember"] = false;
    structInfo["needTempVariable"] = false;
    structInfo["genStructWrapperF"] = !isBinaryStruct(structType);
    structInfo["noSharedMem"] = (findAnnotation(structType, NO_SHARED_ANNOTATION) != nullptr);

    setTemplateComments(structType, structInfo);

    // set struct members template data
    data_list members;
    assert(dynamic_cast<DataList *>(structInfo["members"].get().get()));
    data_list baseMembers = dynamic_cast<DataList *>(structInfo["members"].get().get())->getlist();
    data_list membersToFree;

    for (StructMember *member : structType->getMembers())
    {
        data_map member_info;

        // find base member template
        for (data_ptr mt : baseMembers)
        {
            if (mt->getmap()["name"]->getvalue() == getOutputName(member))
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
        string memberName = getOutputName(member);

        bool isPtrType = (member->isByref() || trueDataType->isBinary() || trueDataType->isString() ||
                          trueDataType->isList() || trueDataType->isFunction());
        // When forward declaration is used member should be present as pointer to data.
        if (!isPtrType && isMemberDataTypeUsingForwardDeclaration(structType, dataType))
        {
            throw syntax_error(
                format_string("line %d: Struct member shall use byref option. Member is using forward declared type.",
                              member->getFirstLine())
                    .c_str());
        }
        // Handle nullable annotation
        bool isNullable =
            ((findAnnotation(member, NULLABLE_ANNOTATION) != nullptr) &&
             (isPtrType || (trueDataType->isStruct() && (isListStruct(dynamic_cast<StructType *>(trueDataType)) ||
                                                         (isBinaryStruct(dynamic_cast<StructType *>(trueDataType)))))));
        member_info["isNullable"] = isNullable;
        member_info["structElements"] = "";
        member_info["structElementsCount"] = "";
        member_info["noSharedMem"] = (findAnnotation(member, NO_SHARED_ANNOTATION) != nullptr);

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
        if (referencedFrom && !findAnnotation(member, SHARED_ANNOTATION))
        {
            Log::debug("Skipping EncodeDecode member '%s' with paramType '%s' (it's serialized with member '%s').\n",
                       memberName.c_str(), dataType->getName().c_str(), referencedFrom->getName().c_str());
            member_info["coderCall"] = "";
            member_info["serializedViaMember"] = getOutputName(referencedFrom);
        }
        else
        {
            Log::debug("Calling EncodeDecode member '%s' with paramType '%s'.\n", memberName.c_str(),
                       dataType->getName().c_str());

            // Subtemplate setup for read/write struct calls
            bool needTempVariable = false;
            member_info["coderCall"] = getEncodeDecodeCall("data->" + memberName, group, dataType, structType, true,
                                                           member, needTempVariable, false);

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
    info["name"] = getOutputName(unionType);
    info["type"] = "union";

    setTemplateComments(unionType, info);
    // external annotation
    if (findAnnotation(unionType, EXTERNAL_ANNOTATION))
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

data_map CGenerator::getUnionDefinitionTemplateData(Group *group, UnionType *unionType, data_map &unionInfo,
                                                    bool &needUnionsServerFree)
{
    bool needTempVariable = false;
    unionInfo["coderCall"] =
        getEncodeDecodeCall("data->", nullptr, unionType, nullptr, true, nullptr, needTempVariable, false);
    unionInfo["needTempVariable"] = needTempVariable;

    unionInfo["noSharedMem"] = (findAnnotation(unionType, NO_SHARED_ANNOTATION) != nullptr);

    // free unions on server side.
    for (auto unionCase : unionType->getUniqueCases())
    {
        if (!unionCase->caseMemberIsVoid())
        {
            for (auto memberName : unionCase->getMemberDeclarationNames())
            {
                StructMember *unionMember = unionCase->getUnionMemberDeclaration(memberName);
                DataType *trueMemberDataType = unionMember->getDataType()->getTrueDataType();
                if (unionMember->isByref() || trueMemberDataType->isList() || trueMemberDataType->isBinary() ||
                    trueMemberDataType->isString())
                {
                    needUnionsServerFree = true;
                    break;
                }
                else if (trueMemberDataType->isStruct())
                {
                    StructType *structType = dynamic_cast<StructType *>(trueMemberDataType);
                    assert(structType);
                    set<DataType *> loopDetection;
                    if (structType->containStringMember() || structType->containListMember() ||
                        containsByrefParamToFree(structType, loopDetection))
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

void CGenerator::setUnionMembersTemplateData(UnionType *unionType, data_map &unionInfo)
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
                string unionMemberName = getOutputName(unionMember);
                DataType *unionMemberType = unionMember->getDataType()->getTrueDataType();

                // Check if member is byRef type. Add "*" for type and allocate space for data on server side.
                if (unionMember->isByref() && (unionMemberType->isStruct() || unionMemberType->isUnion() ||
                                               unionMemberType->isScalar() || unionMemberType->isEnum()))
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

string CGenerator::getUnionMembersData(UnionType *unionType, string indent)
{
    // Data are parsed from templates data and returned as string.
    data_map data;
    data["unionMembersDeclaration"] = m_templateData["unionMembersDeclaration"];
    setUnionMembersTemplateData(unionType, data);
    data["info"] = data;

    // there is always one indent "...."
    indent += "    ";

    // code to parse
    string command = "{$addIndent('" + indent + "', unionMembersDeclaration(info))}";

    // parse
    return parse(command, data);
}

void CGenerator::setTemplateComments(Symbol *symbol, data_map &symbolInfo)
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
    return (isServerNullParam(param) ||
            ((paramTrueDataType->isScalar() || paramTrueDataType->isEnum()) && param->getDirection() != kInDirection));
}

bool CGenerator::isNullableParam(StructMember *param)
{
    return (findAnnotation(param, NULLABLE_ANNOTATION) && isPointerParam(param));
}

data_map CGenerator::getFunctionBaseTemplateData(Group *group, FunctionBase *fn)
{
    data_map info;
    Symbol *fnSymbol = dynamic_cast<Symbol *>(fn);

    // reset list numbering.
    listCounter = 0;

    info["isOneway"] = fn->isOneway();
    info["isReturnValue"] = !fn->isOneway();
    info["isSendValue"] = false;
    setTemplateComments(fnSymbol, info);
    info["needTempVariableServer"] = false;
    info["needTempVariableClient"] = false;
    info["needNullVariableOnServer"] = false;

    /* Is function declared as external? */
    info["isNonExternalFunction"] = !findAnnotation(fnSymbol, EXTERNAL_ANNOTATION);

    // Get return value info
    data_map returnInfo;
    returnInfo["type"] = getTypeInfo(fn->getReturnType(), true);
    StructMember *structMember = fn->getReturnStructMemberType();
    DataType *dataType = fn->getReturnType();
    DataType *trueDataType = dataType->getTrueDataType();
    if (!trueDataType->isVoid())
    {
        string result = "result";
        bool needTempVariable = false;
        setCallingFreeFunctions(structMember, returnInfo, true);
        string extraPointer = getExtraPointerInReturn(dataType);
        string resultVariable = extraPointer + returnSpaceWhenNotEmpty(extraPointer) + result;
        if (findAnnotation(structMember, NULLABLE_ANNOTATION) == nullptr)
        {
            returnInfo["isNullable"] = false;
            returnInfo["nullableName"] = "";
        }
        else
        {
            returnInfo["isNullable"] = true;
            returnInfo["nullableName"] = result;
        }
        bool isShared = (findAnnotation(structMember, SHARED_ANNOTATION) != nullptr);
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
        returnInfo["coderCall"] =
            getEncodeDecodeCall(result, group, dataType, nullptr, false, structMember, needTempVariable, true);
        returnInfo["shared"] = isShared;
        resultVariable = getTypenameName(dataType, resultVariable);
        info["needTempVariableClient"] = needTempVariable;
        returnInfo["resultVariable"] = resultVariable;
        returnInfo["errorReturnValue"] = getErrorReturnValue(fn);
        returnInfo["isNullReturnType"] = (!trueDataType->isScalar() && !trueDataType->isEnum());
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
        string name = getOutputName(param);

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
        if (referencedFrom != nullptr && findAnnotation(referencedFrom, SHARED_ANNOTATION) == nullptr)
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
                paramInfo["referencedName"] = getOutputName(referencedFrom);
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
            string lengthParam = getAnnStringValue(param, LENGTH_ANNOTATION);
            if (!lengthParam.empty())
            {
                Symbol *symbol = fn->getParameters().getScope().getSymbol(lengthParam, false);
                if (symbol != nullptr)
                {
                    StructMember *st = dynamic_cast<StructMember *>(symbol);
                    assert(st);
                    if (isNullableParam(st))
                    {
                        paramInfo["lengthName"] = getOutputName(symbol);
                    }
                }
            }

            // Special case when scalar variables are @nullable
            string nullableName = getOutputName(param);
            paramInfo["nullableName"] = nullableName;
            if (paramTrueType->isScalar() || paramTrueType->isEnum())
            {
                string nullVariable = "_" + nullableName;
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
            string maxLengthName = getAnnStringValue(param, MAX_LENGTH_ANNOTATION);
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
        bool isShared = (isPointerParam(param) && findAnnotation(param, SHARED_ANNOTATION) != nullptr);
        paramInfo["shared"] = isShared;
        string encodeDecodeName;
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

        paramInfo["variable"] = getTypenameName(paramType, name);
        paramInfo["name"] = name;

        Log::debug("Calling EncodeDecode param %s with paramType %s.\n", param->getName().c_str(),
                   paramType->getName().c_str());
        paramInfo["coderCall"] = getEncodeDecodeCall(encodeDecodeName, group, paramType, &fn->getParameters(), false,
                                                     param, needTempVariable, true);

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
            (isNeedCallFree(paramType) || paramInfo["firstFreeingCall1"]->getmap()["freeName"]->getvalue() != ""))
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

    return info;
}

data_map CGenerator::getFunctionTemplateData(Group *group, Function *fn)
{
    data_map info;

    info = getFunctionBaseTemplateData(group, fn);

    // Ignore function shim code. Will be serialized through common function.
    bool useCommonFunction = false;
    if (fn->getFunctionType())
    {
        int similarFunctions = 0;
        for (Interface *interface : group->getInterfaces())
        {
            for (Function *function : interface->getFunctions())
            {
                if (fn->getFunctionType() == function->getFunctionType())
                {
                    ++similarFunctions;
                }
            }
        }

        if (similarFunctions > 1)
        {
            useCommonFunction = true;
        }
    }

    if (useCommonFunction)
    {
        std::string callbackFName = getOutputName(fn->getFunctionType());
        if (!group->getName().empty())
        {
            callbackFName += "_" + group->getName();
        }
        info["isCallback"] = true;
        info["callbackFName"] = callbackFName;
        info["serviceId"] = fn->getInterface()->getUniqueId();
    }
    else
    {
        info["isCallback"] = false;
        string serverProto = getFunctionServerCall(fn);
        info["serverPrototype"] = serverProto;
        info["serviceId"] = "";
    }

    string proto = getFunctionPrototype(group, fn);
    info["prototype"] = proto;
    info["name"] = getOutputName(fn);
    info["id"] = fn->getUniqueId();

    return info;
}

data_map CGenerator::getFunctionTypeTemplateData(Group *group, FunctionType *fn)
{
    data_map info;
    std::string name = getOutputName(fn);
    // group specific common function name
    if (!group->getName().empty())
    {
        name += "_" + group->getName();
    }

    // set parameters name
    int j = 0;
    for (StructMember *structMember : fn->getParameters().getMembers())
    {
        if (structMember->getName().empty())
        {
            structMember->setName("param" + boost::lexical_cast<std::string>(j++));
        }
    }

    string proto = getFunctionPrototype(group, fn, name);
    info = getFunctionBaseTemplateData(group, fn);
    info["prototype"] = proto;
    info["name"] = name;

    data_list functionInfos;
    for (Function *function : fn->getCallbackFuns())
    {
        data_map functionInfo = getFunctionTemplateData(group, function);
        // set serverPrototype function with parameters of common function.
        string serverProto = getFunctionServerCall(function, fn);
        functionInfo["serverPrototype"] = serverProto;
        functionInfos.push_back(functionInfo);
    }

    info["functions"] = functionInfos;

    return info;
}

void CGenerator::setSymbolDataToSide(const Symbol *symbolType, const set<_param_direction> directions,
                                     data_list &toClient, data_list &toServer, data_map &dataMap)
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
            bool inOut = directions.count(kInoutDirection);
            bool ret = directions.count(kReturn);

            Log::info("Symbol %s has directions: in:%d, out:%d, inOut:%d, ret:%d\n", symbolType->getName().c_str(), in,
                      out, inOut, ret);

            if (inOut || (in && (ret || out)))
            {
                direction = kInOut;
            }
            else if (ret || out)
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
            throw internal_error(format_string("Symbol: %s is not structure or function parameter.",
                                               symbolType->getDescription().c_str()));
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
            throw runtime_error("internal error: unexpected parameter direction");
    }
}

data_map CGenerator::getTypeInfo(DataType *t, bool isFunction)
{
    data_map info;
    info["isNotVoid"] = make_data(t->getDataType() != DataType::kVoidType);
    return info;
}

string CGenerator::getErrorReturnValue(FunctionBase *fn)
{
    Symbol *symbol = dynamic_cast<Symbol *>(fn);
    assert(symbol);

    Value *returnVal = getAnnValue(symbol, ERROR_RETURN_ANNOTATION);
    DataType *dataType = fn->getReturnType()->getTrueDataType();
    if (returnVal)
    {
        if (returnVal->toString().empty())
        {
            throw semantic_error(format_string("Expected value for @%s annotation for function on line %d",
                                               ERROR_RETURN_ANNOTATION, symbol->getLocation().m_firstLine));
        }
        else if (dataType->isString())
        {
            return (dataType->isUString() ? "(unsigned char *)" : "(char*)") + returnVal->toString();
        }
        else if (dataType->isScalar())
        {
            BuiltinType *builtinType = dynamic_cast<BuiltinType *>(dataType);
            assert(builtinType);
            if (builtinType->getBuiltinType() == BuiltinType::kBoolType)
            {
                IntegerValue *integerValue = dynamic_cast<IntegerValue *>(returnVal);
                assert(integerValue);
                return (integerValue->getValue()) ? "true" : "false";
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
                    return "0xFFU";
                }
                case BuiltinType::kUInt16Type:
                {
                    return "0xFFFFU";
                }
                case BuiltinType::kUInt32Type:
                {
                    return "0xFFFFFFFFU";
                }
                case BuiltinType::kUInt64Type:
                {
                    return "0xFFFFFFFFFFFFFFFFU";
                }
                default:
                {
                    return "-1";
                }
            }
        }
        else if (dataType->isEnum())
        {
            return "(" + getOutputName(fn->getReturnType()) + ") -1";
        }
        else
        {
            return "NULL";
        }
    }
}

string CGenerator::getFunctionServerCall(Function *fn, FunctionType *functionType)
{
    string proto = "";
    if (!fn->getReturnType()->isVoid())
    {
        proto += "result = ";
    }
    proto += getOutputName(fn);
    proto += "(";

    auto params = (functionType) ? functionType->getParameters().getMembers() : fn->getParameters().getMembers();

    if (params.size())
    {
        int n = 0;
        for (auto it : params)
        {
            bool isLast = (n == params.size() - 1);
            DataType *trueDataType = it->getDataType()->getTrueDataType();

            /* Builtin types and function types. */
            if (((trueDataType->isScalar()) || trueDataType->isEnum()) && it->getDirection() != kInDirection &&
                findAnnotation(it, NULLABLE_ANNOTATION))
            {
                // On server side is created new variable for handle null : "_" + name
                proto += "_";
            }
            else if ((it->getDirection() != kInDirection) && (((trueDataType->isScalar()) || trueDataType->isEnum()) ||
                                                              (findAnnotation(it, SHARED_ANNOTATION))))

            {
                proto += "&";
            }
            proto += getOutputName(it);

            if (!isLast)
            {
                proto += ", ";
            }
            ++n;
        }
    }
    return proto + ");";
}

string CGenerator::getFunctionPrototype(Group *group, FunctionBase *fn, std::string name)
{
    DataType *dataTypeReturn = fn->getReturnType();
    string proto = getExtraPointerInReturn(dataTypeReturn);
    if (proto == "*")
    {
        proto += " ";
    }

    Symbol *symbol = dynamic_cast<Symbol *>(fn);
    assert(symbol);

    FunctionType *funType = dynamic_cast<FunctionType *>(fn);
    if (name.empty())
    {
        string functionName = getOutputName(symbol);
        if (funType) /* Need add '(*name)' for function type definition. */
        {
            proto += "(*" + functionName + ")";
        }
        else /* Use function name only. */
        {
            proto += functionName;
        }
    }
    else
    {
        proto += name;
    }

    proto += "(";

    auto params = fn->getParameters().getMembers();
    // add interface id and function id parameters for common callbacks shim code function
    if (!name.empty())
    {
        proto += "uint32_t serviceID, uint32_t functionID";
        if (params.size() > 0)
        {
            proto += ", ";
        }
    }

    if (params.size())
    {
        int n = 0;
        for (auto it : params)
        {
            bool isLast = (n == params.size() - 1);
            string paramSignature = getOutputName(it);
            DataType *dataType = it->getDataType();
            DataType *trueDataType = dataType->getTrueDataType();

            /* Add '*' to data types. */
            if (((trueDataType->isBuiltin() || trueDataType->isEnum()) &&
                 (it->getDirection() != kInDirection && !trueDataType->isString())) ||
                (trueDataType->isFunction() &&
                 (it->getDirection() == kOutDirection || it->getDirection() == kInoutDirection)))
            {
                paramSignature = "* " + paramSignature;
            }
            else
            {
                string directionPointer = getExtraDirectionPointer(it);
                paramSignature = directionPointer + returnSpaceWhenNotEmpty(directionPointer) + paramSignature;
            }
            paramSignature = getTypenameName(dataType, paramSignature);

            if (!(m_def->hasProgramSymbol() &&
                  (findAnnotation(m_def->getProgramSymbol(), NO_CONST_PARAM) ||
                   ((funType && findAnnotation(funType, NO_CONST_PARAM)) ||
                    (!funType && findAnnotation(dynamic_cast<Function *>(fn), NO_CONST_PARAM))) ||
                   findAnnotation(it, NO_CONST_PARAM))))
            {
                if ((dataType->isString() || dataType->isFunction() || trueDataType->isStruct() ||
                     trueDataType->isList() || trueDataType->isArray() || trueDataType->isBinary() ||
                     trueDataType->isUnion()) &&
                    it->getDirection() == kInDirection)
                {
                    bool pass = true;
                    if (trueDataType->isArray())
                    {
                        ArrayType *arrayType = dynamic_cast<ArrayType *>(trueDataType);
                        assert(arrayType);
                        DataType *elementType = arrayType->getElementType()->getTrueDataType();
                        if (elementType->isArray() || elementType->isString() || elementType->isList() ||
                            elementType->isBinary())
                        {
                            pass = false;
                        }
                    }
                    if (trueDataType->isList())
                    {
                        ListType *listType = dynamic_cast<ListType *>(trueDataType);
                        assert(listType);
                        DataType *elementType = listType->getElementType()->getTrueDataType();
                        if (elementType->isArray() || elementType->isString() || elementType->isList() ||
                            elementType->isBinary())
                        {
                            pass = false;
                        }
                    }
                    if (pass)
                    {
                        proto += "const ";
                    }
                }
            }

            DataType *trueContainerDataType = dataType->getTrueContainerDataType();
            if (trueContainerDataType->isStruct())
            {
                StructType *structType = dynamic_cast<StructType *>(trueContainerDataType);
                assert(structType);
                // Todo: Need check if members are/aren't shared.
                if (group != nullptr)
                {
                    const set<_param_direction> directions = group->getSymbolDirections(structType);
                    if (!findAnnotation(it, SHARED_ANNOTATION) &&
                        (directions.count(kInoutDirection) &&
                         (directions.count(kOutDirection) || directions.count(kReturn))))
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

string CGenerator::generateIncludeGuardName(const string &filename)
{
    string guard;

    // strip directory prefixes
    string fileNoPath = filename;
    size_t found = filename.find_last_of("/\\");
    if (found != string::npos)
    {
        string fileNoPath = filename.substr(found + 1);
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

string CGenerator::getTypenameName(DataType *t, const string &name)
{
    string returnName;
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
                returnName = getOutputName(t);
                returnName += returnSpaceWhenNotEmpty(name) + name;
            }
            else
            {
                returnName += "union\n    {\n";
                returnName += getUnionMembersData(unionType, "    ");
                returnName += "\n    } " + name;
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
            returnName = getOutputName(t);
            returnName += returnSpaceWhenNotEmpty(name) + name;
            break;
        }
        default:
            throw internal_error(format_string("In getTypenameName: unknown data type: %s value:%d",
                                               t->getName().c_str(), t->getDataType()));
    }
    return returnName;
}

string CGenerator::getBuiltinTypename(const BuiltinType *t)
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
        case BuiltinType::kUStringType:
            return "unsigned char*";            
        case BuiltinType::kBinaryType:
            return "uint8_t *";
        default:
            throw internal_error("unknown builtin type");
    }
}

void CGenerator::getEncodeDecodeBuiltin(Group *group, BuiltinType *t, data_map &templateData, StructType *structType,
                                        StructMember *structMember, bool isFunctionParam)
{
    templateData["decode"] = m_templateData["decodeBuiltinType"];
    templateData["encode"] = m_templateData["encodeBuiltinType"];

    if (t->isString())
    {
        templateData["checkStringNull"] = false;
        templateData["withoutAlloc"] = ((structMember->getDirection() == kInoutDirection) ||
                                        (structType && group->getSymbolDirections(structType).count(kInoutDirection))) ?
                                           true :
                                           false;
        if (!isFunctionParam)
        {
            templateData["stringAllocSize"] = getOutputName(structMember) + "_len";
            templateData["stringLocalName"] = getOutputName(structMember);
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
                templateData["stringLocalName"] = getOutputName(structMember);
                templateData["stringAllocSize"] = getAnnStringValue(structMember, MAX_LENGTH_ANNOTATION);
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
        templateData["builtinTypeName"] = t->isUString() ? "unsigned char*" : "char*";
    }
    else
    {
        templateData["builtinType"] = "kNumberType";
    }
}

data_map CGenerator::getEncodeDecodeCall(const string &name, Group *group, DataType *t, StructType *structType,
                                         bool inDataContainer, StructMember *structMember, bool &needTempVariable,
                                         bool isFunctionParam)
{
    // prepare data for template
    data_map templateData;
    string_vector params = string_vector(1, "");
    templateData["freeingCall"] = make_template("", &params);
    templateData["inDataContainer"] = inDataContainer;
    templateData["isElementArrayType"] = false;
    data_map defMemberAllocation;
    templateData["memberAllocation"] = defMemberAllocation;
    // name used for serializing/deserializing current data type.
    string localName;

    int pos = name.rfind("*");
    if ((pos == 0 || pos == 1) && (t->getTrueDataType()->isStruct() || t->getTrueDataType()->isUnion()) &&
        inDataContainer == false)
    {
        localName = name.substr(1, name.length());
    }
    else
    {
        localName = name;
    }

    templateData["freeName"] = localName;

    /* Check if member should be serialized as shared. */
    if ((structMember && findAnnotation(structMember, SHARED_ANNOTATION)) ||
        (structType && findAnnotation(structType, SHARED_ANNOTATION)))
    {
        templateData["funcParam"] = (structType) ? true : false;
        templateData["InoutOutDirection"] = (structMember && (structMember->getDirection() == kOutDirection ||
                                                              structMember->getDirection() == kInoutDirection)) ?
                                                true :
                                                false;
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
                    if (findAnnotation(m, NO_SHARED_ANNOTATION))
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
                        if (findAnnotation(m, NO_SHARED_ANNOTATION))
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

    // Check if member is byRef type. Add "*" for type and allocate space for data on server side.
    if (structMember && structMember->isByref() && !isFunctionParam &&
        (t->isStruct() || t->isUnion() || t->isScalar() || t->isEnum()))
    {
        templateData["freeingCall2"] = m_templateData["freeData"];
        templateData["memberAllocation"] = allocateCall(name, t);
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
            return getEncodeDecodeCall(name, group, aliasType->getElementType(), structType, inDataContainer,
                                       structMember, needTempVariable, isFunctionParam);
        }
        case DataType::kArrayType:
        {
            static uint8_t arrayCounter;
            ArrayType *arrayType = dynamic_cast<ArrayType *>(t);
            assert(arrayType);
            DataType *elementType = arrayType->getElementType();
            DataType *trueElementType = elementType->getTrueDataType();

            string arrayName = name;
            templateData["decode"] = m_templateData["decodeArrayType"];
            templateData["encode"] = m_templateData["encodeArrayType"];

            // To improve code serialization/deserialization for scalar types when BasicCodec is used.
            templateData["builtinTypeName"] =
                ((m_def->getCodecType() != InterfaceDefinition::kBasicCodec) || trueElementType->isBool()) ?
                    "" :
                    getScalarTypename(elementType);

            giveBracesToArrays(arrayName);
            templateData["forLoopCount"] = format_string("arrayCount%d", arrayCounter);
            templateData["protoNext"] =
                getEncodeDecodeCall(format_string("%s[arrayCount%d]", arrayName.c_str(), arrayCounter++), group,
                                    elementType, structType, true, structMember, needTempVariable, isFunctionParam);
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
            string typeName = getOutputName(t);
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
            templateData["callbacksCount"] = callbacks.size();
            if (callbacks.size() > 1)
            {
                templateData["callbacks"] = "_" + funType->getName();
            }
            else if (callbacks.size() == 1)
            {
                templateData["callbacks"] = callbacks[0]->getName();
            }
            else
            {
                throw semantic_error(format_string("line %d: Function has function type parameter (callback "
                                                   "parameter), but in IDL is missing function definition, which can "
                                                   "be passed there.",
                                                   structMember->getFirstLine())
                                         .c_str());
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

            bool isInOut = ((structMember->getDirection() == kInoutDirection) ||
                            (!isFunctionParam && group->getSymbolDirections(structType).count(kInoutDirection)));

            bool isTopDataType = (isFunctionParam && structMember->getDataType()->getTrueDataType() == t);

            // Because cpptempl don't know do correct complicated conditions like
            // if(a || (b && c))
            templateData["useMallocOnClientSide"] = (!isInOut && !isTopDataType);

            templateData["mallocSizeType"] = getTypenameName(elementType, "");
            templateData["mallocType"] = getTypenameName(elementType, "*");
            templateData["needFreeingCall"] =
                (generateServerFreeFunctions(structMember) && isNeedCallFree(elementType));

            // To improve code serialization/deserialization for scalar types when BasicCodec is used.
            templateData["builtinTypeName"] =
                ((m_def->getCodecType() != InterfaceDefinition::kBasicCodec) || trueElementType->isBool()) ?
                    "" :
                    getScalarTypename(elementType);

            if (generateServerFreeFunctions(structMember))
            {
                templateData["freeingCall"] = m_templateData["freeList"];
            }
            templateData["decode"] = m_templateData["decodeListType"];
            templateData["encode"] = m_templateData["encodeListType"];

            string nextName;

            // Size variable should have same prefix as variable name.
            auto sizePrefix = name.rfind('.');
            if (sizePrefix == string::npos)
            {
                sizePrefix = name.rfind('>');
            }
            string size = "";
            if (sizePrefix != string::npos)
            {
                size = name.substr(0, sizePrefix + 1);
            }
            templateData["pointerScalarTypes"] = false;
            templateData["constantVariable"] = false;

            if (listType->hasLengthVariable())
            {
                templateData["hasLengthVariable"] = true;

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
                        // it is just number.
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
                templateData["sizeTemp"] = string("lengthTemp_") + to_string(listCounter);
                templateData["dataTemp"] = string("dataTemp_") + to_string(listCounter);
                string maxSize = getAnnStringValue(structMember, MAX_LENGTH_ANNOTATION);
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
                templateData["forLoopCount"] = string("listCount") + to_string(listCounter);
                templateData["isElementArrayType"] = trueElementType->isArray();
                ++listCounter;
            }
            else
            {
                templateData["hasLengthVariable"] = false;
                if (sizePrefix != string::npos)
                {
                    string usedName = name.substr(sizePrefix + 1, name.size() - sizePrefix - 1);
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
            templateData["protoNext"] = getEncodeDecodeCall(nextName, group, elementType, structType, true,
                                                            structMember, needTempVariable, isFunctionParam);
            break;
        }
        case DataType::kStructType:
        {
            // needDealloc(templateData, t, structType, structMember);
            string typeName = getOutputName(t);
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
            assert(unionType);

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
                templateData["typeName"] = getOutputName(t);
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

                string disriminatorSeparator = "";

                /* Get discriminator separator for encapsulated unions. data->variable_name.case */
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
                        // Create phony encode/decode values for void function,
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
                            string unionCaseName = name + disriminatorSeparator + getOutputName(memberDeclaration);
                            memberData["coderCall"] =
                                getEncodeDecodeCall(unionCaseName, group, memberDeclaration->getDataType(), structType,
                                                    true, structMember, casesNeedTempVariable, isFunctionParam);
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

void CGenerator::giveBracesToArrays(string &name)
{
    if (name.size() && name.substr(0, 1) == "*")
    {
        name = "(" + name + ")";
    }
}

string CGenerator::getExtraPointerInReturn(DataType *dataType)
{
    DataType *trueDataType = dataType->getTrueDataType();
    if (trueDataType->isStruct() || trueDataType->isArray() || trueDataType->isUnion())
    {
        return "*";
    }

    return "";
}

string CGenerator::getExtraDirectionPointer(StructMember *structMember)
{
    DataType *dataType = structMember->getDataType();
    DataType *trueDataType = dataType->getTrueDataType();
    _param_direction structMemberDir = structMember->getDirection();
    string result;
    if (structMemberDir == kOutDirection) // between out and inout can be differences in future. Maybe not.
    {
        if (!trueDataType->isBuiltin() && !trueDataType->isEnum() && !trueDataType->isList() &&
            !trueDataType->isArray())
        {
            result = "*";
        }
        if (findAnnotation(structMember, SHARED_ANNOTATION))
        {
            result += "*";
        }
    }
    else if (structMemberDir == kInoutDirection)
    {
        if (!trueDataType->isBuiltin() && !trueDataType->isEnum() && !trueDataType->isList() &&
            !trueDataType->isArray())
        {
            result = "*";
        }
        if (findAnnotation(structMember, SHARED_ANNOTATION))
        {
            result += "*";
        }
    }
    else
    {
        if ((trueDataType->isStruct() || trueDataType->isUnion()) ||
            (trueDataType->isScalar() && findAnnotation(structMember, SHARED_ANNOTATION)))
        {
            result = "*";
        }
    }

    return result;
}

data_map CGenerator::firstAllocOnReturnWhenIsNeed(string name, DataType *dataType)
{
    DataType *trueDataType = dataType->getTrueDataType();
    if ((trueDataType->isArray() || trueDataType->isStruct() || trueDataType->isUnion()) &&
        !findAnnotation(dataType, SHARED_ANNOTATION))
    {
        return allocateCall(name, dataType);
    }

    data_map r;
    return r;
}

data_map CGenerator::firstAllocOnServerWhenIsNeed(string name, StructMember *structMember)
{
    DataType *dataType = structMember->getDataType();
    DataType *trueDataType = dataType->getTrueDataType();
    _param_direction structMemberDir = structMember->getDirection();
    if (!findAnnotation(structMember, SHARED_ANNOTATION))
    {
        if (structMemberDir == kInoutDirection)
        {
            if (!trueDataType->isBuiltin() && !trueDataType->isEnum() && !trueDataType->isList() &&
                !trueDataType->isArray())
            {
                return allocateCall(name, structMember);
            }
        }
        else if (structMemberDir == kInDirection)
        {
            if (trueDataType->isStruct() || trueDataType->isUnion())
            {
                return allocateCall(name, structMember);
            }
        }
        else if (structMember->getDirection() == kOutDirection)
        {
            if (!trueDataType->isBuiltin() && !trueDataType->isEnum() && !trueDataType->isArray())
            {
                return allocateCall(name, structMember);
            }
            else if (trueDataType->isString())
            {
                if (!findAnnotation(structMember, MAX_LENGTH_ANNOTATION))
                {
                    Symbol *symbol = structMember;
                    assert(symbol);
                    throw semantic_error(
                        format_string("For out string variable '%s' on line '%d' max_length annotation has to be set.",
                                      symbol->getName().c_str(), symbol->getLocation().m_firstLine));
                }
                data_map returnValue = allocateCall(name, structMember);
                returnValue["isOutChar"] = true;
                return returnValue;
            }
        }
    }

    data_map r;
    return r;
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
            return structType->containListMember() || structType->containStringMember() ||
                   containsByrefParamToFree(structType, loopDetection);
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

void CGenerator::setCallingFreeFunctions(Symbol *symbol, data_map &info, bool returnType)
{
    StructMember *structMember = dynamic_cast<StructMember *>(symbol);
    assert(structMember);
    DataType *trueDataType = structMember->getDataType()->getTrueDataType();
    data_map firstFreeingCall1;
    firstFreeingCall1["firstFreeingCall"] = "";
    firstFreeingCall1["freeName"] = "";
    // When true then function parameter, else return type
    if (!findAnnotation(symbol, SHARED_ANNOTATION) && generateServerFreeFunctions(structMember))
    {
        if (!returnType)
        {
            switch (trueDataType->getDataType())
            {
                case DataType::kStructType:
                case DataType::kUnionType:
                {
                    string name = getOutputName(structMember, false);
                    firstFreeingCall1["firstFreeingCall"] = m_templateData["freeData"];
                    firstFreeingCall1["freeName"] = name;
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
}

data_map CGenerator::allocateCall(const string &name, Symbol *symbol)
{
    StructMember *structMember = dynamic_cast<StructMember *>(symbol);
    DataType *dataType;
    data_map alloc;
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
    string size;
    string chSize;
    if ((trueDataType->isList() || trueDataType->isString()) && structMember)
    {
        size = getAnnStringValue(structMember, MAX_LENGTH_ANNOTATION);
        if (size != "")
        {
            if (dataType->isString())
            {
                chSize = size;
                size = "(" + size + " + 1)";
            }
            size += " * ";
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
    string typeValue;
    string typePointerValue;
    if (!dataType->isString())
    {
        typeValue = getTypenameName(dataType, "");
        typePointerValue = getTypenameName(dataType, "*");
    }
    else
    {
        typeValue = "char";
        typePointerValue = dataType->isUString() ? "unsigned char*" : "char *";
    }

    alloc["name"] = name.c_str();
    alloc["typePointerValue"] = typePointerValue.c_str();
    alloc["size"] = size.c_str();
    alloc["chSize"] = chSize.c_str();
    alloc["typeValue"] = typeValue.c_str();
    alloc["isOutChar"] = false;
    return alloc;
}

string CGenerator::returnSpaceWhenNotEmpty(const string &param)
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
                    for (string unionCaseName : unionCase->getMemberDeclarationNames())
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
                    for (string unionCaseName : unionCase->getMemberDeclarationNames())
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
                if ((structMember->isByref() && !findAnnotation(structMember, SHARED_ANNOTATION)) ||
                    containsByrefParamToFree(structMember->getDataType(), dataTypes))
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
                if ((structMember->isByref() && !findAnnotation(structMember, SHARED_ANNOTATION)) ||
                    containsByrefParamToFree(structMember->getDataType(), dataTypes))
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
    return (structMember == nullptr || findAnnotation(structMember, RETAIN_ANNOTATION) == nullptr);
}

void CGenerator::setNoSharedAnn(Symbol *parentSymbol, Symbol *childSymbol)
{
    if (Annotation *ann = findAnnotation(childSymbol, NO_SHARED_ANNOTATION))
    {
        parentSymbol->addAnnotation(*ann);
    }
}

bool CGenerator::setDiscriminatorTemp(UnionType *unionType, StructType *structType, StructMember *structMember,
                                      bool isFunctionParam, data_map &templateData)
{
    bool needTempVariable = false;
    if (structType)
    {
        string discriminatorName;
        Symbol *disSymbol;

        if (unionType->isNonEncapsulatedUnion())
        {
            discriminatorName = getAnnStringValue(structMember, DISCRIMINATOR_ANNOTATION);
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

string CGenerator::getScalarTypename(DataType *dataType)
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

string CGenerator::getDirection(_param_direction direction)
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

void CGenerator::initCReservedWords()
{
    // C specific words
    reserverdWords.insert("if");
    reserverdWords.insert("else");
    reserverdWords.insert("switch");
    reserverdWords.insert("case");
    reserverdWords.insert("default");
    reserverdWords.insert("break");
    reserverdWords.insert("int");
    reserverdWords.insert("float");
    reserverdWords.insert("char");
    reserverdWords.insert("double");
    reserverdWords.insert("long");
    reserverdWords.insert("for");
    reserverdWords.insert("while");
    reserverdWords.insert("do");
    reserverdWords.insert("void");
    reserverdWords.insert("goto");
    reserverdWords.insert("auto");
    reserverdWords.insert("signed");
    reserverdWords.insert("const");
    reserverdWords.insert("extern");
    reserverdWords.insert("register");
    reserverdWords.insert("unsigned");
    reserverdWords.insert("return");
    reserverdWords.insert("continue");
    reserverdWords.insert("enum");
    reserverdWords.insert("sizeof");
    reserverdWords.insert("struct");
    reserverdWords.insert("typedef");
    reserverdWords.insert("union");
    reserverdWords.insert("volatile");
    reserverdWords.insert("bool");
    reserverdWords.insert("int8_t");
    reserverdWords.insert("int16_t");
    reserverdWords.insert("int32_t");
    reserverdWords.insert("int64_t");
    reserverdWords.insert("uint8_t");
    reserverdWords.insert("uint16_t");
    reserverdWords.insert("uint32_t");
    reserverdWords.insert("uint64_t");

    // C++ specific words
    reserverdWords.insert("asm");
    reserverdWords.insert("catch");
    reserverdWords.insert("class");
    reserverdWords.insert("const_cast");
    reserverdWords.insert("delete");
    reserverdWords.insert("dynamic_cast");
    reserverdWords.insert("explicit");
    reserverdWords.insert("false");
    reserverdWords.insert("friend");
    reserverdWords.insert("inline");
    reserverdWords.insert("mutable");
    reserverdWords.insert("namespace");
    reserverdWords.insert("new");
    reserverdWords.insert("operator");
    reserverdWords.insert("private");
    reserverdWords.insert("public");
    reserverdWords.insert("protected");
    reserverdWords.insert("reinterpret_cast");
    reserverdWords.insert("static_cast");
    reserverdWords.insert("template");
    reserverdWords.insert("this");
    reserverdWords.insert("throw");
    reserverdWords.insert("true");
    reserverdWords.insert("try");
    reserverdWords.insert("typeid");
    reserverdWords.insert("typename");
    reserverdWords.insert("using");
    reserverdWords.insert("virtual");
    reserverdWords.insert("wchar_t");
}

void CGenerator::checkIfAnnValueIsIntNumberOrIntType(Annotation *ann, StructType *currentStructType)
{
    // skip if value is integer number
    if (ann->getValueObject()->getType() != kIntegerValue)
    {
        string annName = ann->getName();
        int annNameFirstLine = ann->getLocation().m_firstLine;
        string annValue = ann->getValueObject()->toString();

        DataType *trueDataType = nullptr;

        // search in structure scope for member referenced with annotation
        for (StructMember *structMember : currentStructType->getMembers())
        {
            if (structMember->getName().compare(annValue) == 0)
            {
                trueDataType = structMember->getDataType()->getTrueDataType();
                break;
            }
        }

        if (!trueDataType)
        {
            // search in global scope for member referenced with annotation
            Symbol *symbolConst = m_globals->getSymbol(annValue);
            if (symbolConst)
            {
                ConstType *constVar = dynamic_cast<ConstType *>(symbolConst);
                if (constVar)
                {
                    trueDataType = constVar->getDataType();
                }
            }
        }

        if (trueDataType)
        {
            // check if data type is integer type
            if (trueDataType->isScalar())
            {
                BuiltinType *builtinType = dynamic_cast<BuiltinType *>(trueDataType);
                if (builtinType && builtinType->isInt())
                {
                    return;
                }
            }

            throw semantic_error(format_string(
                "line %d: Annotation %s contains reference to non-integer parameter or member %s declared on line %d.",
                annNameFirstLine, annName.c_str(), annValue.c_str(), trueDataType->getLocation().m_firstLine));
        }
        else
        {
            throw semantic_error(format_string("line %d: The parameter or member named by a %s annotation must exist.",
                                               annNameFirstLine, annName.c_str()));
        }
    }
}

void CGenerator::scanStructForAnnotations(StructType *currentStructType, bool isFunction)
{
    // go trough all structure members
    for (StructMember *structMember : currentStructType->getMembers())
    {
        DataType *memberType = structMember->getDataType()->getTrueDataType();
        /* Check non-encapsulated discriminated unions. */
        // looking for references
        Annotation *lengthAnn = findAnnotation(structMember, LENGTH_ANNOTATION);
        Annotation *maxLengthAnn = findAnnotation(structMember, MAX_LENGTH_ANNOTATION);
        if (lengthAnn)
        {
            checkIfAnnValueIsIntNumberOrIntType(lengthAnn, currentStructType);

            if (lengthAnn->getValueObject()->getType() != kIntegerValue)
            {
                StructMember *structMemberRef = NULL;

                // search in structure scope for member referenced with annotation
                Symbol *symbol =
                    currentStructType->getScope().getSymbol(lengthAnn->getValueObject()->toString(), false);
                if (symbol)
                {
                    structMemberRef = dynamic_cast<StructMember *>(symbol);
                    assert(structMemberRef);
                }

                // Verify both the data and length members are the same direction.
                if (!isFunction && structMemberRef && structMember->getDirection() != structMemberRef->getDirection())
                {
                    throw semantic_error(
                        format_string("orig line %d, ref line %d: The parameter named by a length annotation must be "
                                      "the same direction as the data parameter.",
                                      lengthAnn->getLocation().m_firstLine, structMember->getLocation().m_firstLine));
                }
                // Verify using max_length annotation when referenced variable is out.
                else if (isFunction && structMemberRef && structMemberRef->getDirection() == kOutDirection &&
                         !findAnnotation(structMember, MAX_LENGTH_ANNOTATION))
                {
                    throw semantic_error(
                        format_string("orig line %d, ref line %d: The out parameter with set length annotation "
                                      "must have also set max_length annotation",
                                      lengthAnn->getLocation().m_firstLine, structMember->getLocation().m_firstLine));
                }
                // Verify using max_length annotation when referenced variable is inout.
                else if (isFunction && structMemberRef && structMember->getDirection() == kInoutDirection &&
                         structMemberRef->getDirection() == kInoutDirection &&
                         !findAnnotation(structMember, MAX_LENGTH_ANNOTATION))
                {
                    throw semantic_error(
                        format_string("orig line %d, ref line %d: The inout parameter named by a length annotation "
                                      "must have set max_length annotation",
                                      lengthAnn->getLocation().m_firstLine, structMember->getLocation().m_firstLine));
                }
            }

            // Set length variable name.
            if (memberType->isList())
            {
                ListType *memberListType = dynamic_cast<ListType *>(memberType);
                assert(memberListType);
                memberListType->setLengthVariableName(lengthAnn->getValueObject()->toString());
            }
        }

        if (maxLengthAnn)
        {
            checkIfAnnValueIsIntNumberOrIntType(maxLengthAnn, currentStructType);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
