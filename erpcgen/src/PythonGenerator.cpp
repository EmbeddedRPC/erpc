/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "PythonGenerator.hpp"

#include "Logging.hpp"
#include "ParseErrors.hpp"
#include "annotations.h"
#include "format_string.hpp"

#include <algorithm>
#include <set>
#include <sstream>

using namespace erpcgen;
using namespace cpptempl;
using namespace std;

// Templates strings converted from text files by txt_to_c.py.
extern const char *const kPyCoders;
extern const char *const kPyInit;
extern const char *const kPyCommon;
extern const char *const kPyServer;
extern const char *const kPyClient;
extern const char *const kPyInterface;
extern const char *const kPyGlobalInit;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

PythonGenerator::PythonGenerator(InterfaceDefinition *def)
: Generator(def, kPython)
, m_suffixStrip("")
, m_suffixStripSize(0)
{
    /* Set copyright rules. */
    if (m_def->hasProgramSymbol())
    {
        Symbol *program = m_def->getProgramSymbol();
        assert(program);
        setTemplateComments(program, m_templateData);
    }
}

void PythonGenerator::generateOutputFiles(const string &fileName)
{
    // Make sure the package folder is created.
    boost::filesystem::path dir(fileName);
    dir = m_outputDirectory / dir;
    boost::filesystem::create_directories(dir);

    generateInitFile(fileName);
    generateCommonFile(fileName);
    generateClientFile(fileName);
    generateServerFile(fileName);
    generateInterfaceFile(fileName);
}

void PythonGenerator::generateInitFile(string fileName)
{
    fileName += "/__init__.py";
    generateOutputFile(fileName, "py_init", m_templateData, kPyInit);
}

void PythonGenerator::generateCommonFile(string fileName)
{
    fileName += "/common.py";
    generateOutputFile(fileName, "py_common", m_templateData, kPyCommon);
}

void PythonGenerator::generateClientFile(string fileName)
{
    fileName += "/client.py";
    generateOutputFile(fileName, "py_client", m_templateData, kPyClient);
}

void PythonGenerator::generateServerFile(string fileName)
{
    fileName += "/server.py";
    generateOutputFile(fileName, "py_server", m_templateData, kPyServer);
}

void PythonGenerator::generateInterfaceFile(string fileName)
{
    fileName += "/interface.py";
    generateOutputFile(fileName, "py_interface", m_templateData, kPyInterface);
}

void PythonGenerator::generateGlobalInitFile()
{
    /* Generate file with shim code version. */
    generateOutputFile("__init__.py", "py_global_init", m_templateData, kPyGlobalInit);
}

void PythonGenerator::parseSubtemplates()
{
    const char *templateName = "py_coders";
    try
    {
        parse(kPyCoders, m_templateData);
    }
    catch (TemplateException &e)
    {
        throw TemplateException(format_string("Template %s: %s", templateName, e.what()));
    }
}

void PythonGenerator::generate()
{
    data_list empty;
    m_templateData["enums"] = empty;
    m_templateData["aliases"] = empty;
    m_templateData["structs"] = empty;
    m_templateData["unions"] = empty;
    m_templateData["consts"] = empty;

    parseSubtemplates();

    generateGlobalInitFile();

    if (m_def->hasProgramSymbol())
    {
        for (Annotation *anno : getAnnotations(m_def->getProgramSymbol(), PY_TYPES_NAME_STRIP_SUFFIX_ANNOTATION))
        {
            m_suffixStrip = anno->getValueObject()->toString();
            m_suffixStripSize = m_suffixStrip.size();
        }
    }

    findGroupDataTypes();

    makeIncludesTemplateData();

    // makeAliasesTemplateData();

    makeConstTemplateData();

    makeEnumsTemplateData();

    makeFunctionsTemplateData();

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

    initPythonReservedWords();
}

void PythonGenerator::setTemplateComments(Symbol *symbol, data_map &symbolInfo)
{
    symbolInfo["mlComment"] = convertComment(symbol->getMlComment(), kMultilineComment);
    symbolInfo["ilComment"] = convertComment(symbol->getIlComment(), kInlineComment);
}

data_map PythonGenerator::getFunctionTemplateData(Group *group, Function *fn)
{
    (void)group;
    data_map info;
    string proto = getFunctionPrototype(fn);

    info["name"] = getOutputName(fn);
    info["prototype"] = proto;
    info["id"] = fn->getUniqueId();
    info["isOneway"] = fn->isOneway();
    info["isReturnValue"] = !fn->isOneway();
    setTemplateComments(fn, info);

    /* Is function declared as external? */
    info["isNonExternalFunction"] = findAnnotation(fn, EXTERNAL_ANNOTATION) == nullptr;

    // Get return value info
    data_map returnInfo;
    returnInfo["type"] = getTypeInfo(fn->getReturnType());
    // TODO support annotations on function return values
    //    returnInfo["isNullable"] = (fn->getReturnType()->findAnnotation(NULLABLE_ANNOTATION) != nullptr);
    info["returnValue"] = returnInfo;

    // get function parameter info
    auto fnParams = fn->getParameters().getMembers();
    data_list params;
    data_list inParams;
    data_list outParams;
    for (StructMember *param : fnParams)
    {
        data_map paramInfo;
        DataType *paramType = param->getDataType();
        DataType *trueDataType = paramType->getTrueDataType();
        string name = getOutputName(param);
        paramInfo["name"] = name;
        paramInfo["type"] = getTypeInfo(paramType);

        bool isNullable = ((findAnnotation(param, NULLABLE_ANNOTATION) != nullptr) &&
                           (trueDataType->isString() || (!trueDataType->isBuiltin() && !trueDataType->isEnum())));
        paramInfo["isNullable"] = isNullable;

        // Skip data serialization for variables placed as @length value for lists.
        // These prevent to serialized data twice.
        StructMember *referencedFrom = findParamReferencedFromAnn(fnParams, name, LENGTH_ANNOTATION);
        paramInfo["discriminatorForMember"] = "";

        if (referencedFrom)
        {
            paramInfo["lengthForMember"] = getOutputName(referencedFrom);
        }
        else
        {
            paramInfo["lengthForMember"] = "";
            // Skip data serialization for variables used as discriminator for unions.
            // These prevent to serialized data twice.
            referencedFrom = findParamReferencedFromUnion(fnParams, name);
            if (referencedFrom)
            {
                paramInfo["discriminatorForMember"] = getOutputName(referencedFrom);
            }
        }

        paramInfo["serializedViaMember"] = (referencedFrom) ? getOutputName(referencedFrom) : "";

        /* Necessary for handling non-discriminated unions */
        paramInfo["discriminator"] = getAnnStringValue(param, DISCRIMINATOR_ANNOTATION);

        _param_direction dir = param->getDirection();
        switch (dir)
        {
            case kInDirection:
                paramInfo["direction"] = "in";
                inParams.push_back(paramInfo);
                break;
            case kOutDirection:
                paramInfo["direction"] = "out";
                outParams.push_back(paramInfo);
                break;
            case kInoutDirection:
                paramInfo["direction"] = "inout";
                inParams.push_back(paramInfo);
                outParams.push_back(paramInfo);
                break;
            default:
                paramInfo["direction"] = "none";
        }

        params.push_back(paramInfo);
    }

    info["parameters"] = params;
    info["inParameters"] = inParams;
    info["outParameters"] = outParams;

    return info;
}

string PythonGenerator::getFunctionPrototype(Function *fn)
{
    string proto = getOutputName(fn);
    proto += "(self";

    auto params = fn->getParameters().getMembers();
    if (params.size())
    {
        for (auto it : params)
        {
            // Skip data serialization for variables placed as @length value for lists.
            if (findParamReferencedFromAnn(params, getOutputName(it), LENGTH_ANNOTATION))
            {
                continue;
            }

            proto += ", ";
            proto += getOutputName(it);
        }
    }
    proto += ")";
    return proto;
}

void PythonGenerator::makeConstTemplateData()
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
            constInfo["name"] = getOutputName(constVar);

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
            setTemplateComments(constVar, constInfo);
            Log::info("Name=%s\tType=%s\tValue=%s\n", constVar->getName().c_str(),
                      constVar->getDataType()->getName().c_str(), constVar->getValue()->toString().c_str());
            consts.push_back(constInfo);
        }
    }
    m_templateData["consts"] = consts;
}

void PythonGenerator::makeEnumsTemplateData()
{
    Log::info("Enums:\n");
    data_list enums;
    int n = 0;
    for (auto it : getDataTypesFromSymbolScope(m_globals, DataType::kEnumType))
    {
        EnumType *enumType = dynamic_cast<EnumType *>(it);
        assert(enumType);
        if (!findAnnotation(enumType, EXTERNAL_ANNOTATION))
        {
            Log::info("%d: %s\n", n, enumType->getName().c_str());
            data_map enumInfo;
            enumInfo["name"] = filterName(getOutputName(enumType));
            enumInfo["members"] = getEnumMembersTemplateData(enumType);
            setTemplateComments(enumType, enumInfo);
            enums.push_back(enumInfo);
            ++n;
        }
    }
    m_templateData["enums"] = enums;
}

data_list PythonGenerator::getEnumMembersTemplateData(EnumType *enumType)
{
    int j = 0;
    data_list enumMembersList;
    for (auto member : enumType->getMembers())
    {
        assert(member->hasValue());
        data_map enumMember;
        enumMember["name"] = getOutputName(member);
        enumMember["value"] = member->getValue();
        Log::info("    %d: %s = %d\n", j, member->getName().c_str(), member->getValue());
        setTemplateComments(member, enumMember);
        enumMembersList.push_back(enumMember);
        ++j;
    }
    return enumMembersList;
}

void PythonGenerator::makeAliasesTemplateData()
{
    Log::info("Type definition:\n");
    data_list aliases;
    int n = 0;
    for (auto it : getDataTypesFromSymbolScope(m_globals, DataType::kAliasType))
    {
        AliasType *aliasType = dynamic_cast<AliasType *>(it);
        assert(aliasType);
        //        Annotation *externAnnotation = aliasType->findAnnotation(EXTERNAL_ANNOTATION);
        //        if (!externAnnotation)
        //        {
        Log::info("%d: ", n);
        data_map aliasInfo;
        DataType *elementDataType = aliasType->getElementType();
        DataType *trueDataType = elementDataType->getTrueDataType();

        // Only generate aliases for enums, unions and structs in Python.
        if (!(trueDataType->isEnum() || trueDataType->isUnion() || trueDataType->isStruct()))
        {
            continue;
        }

        string realType = getOutputName(aliasType);
        Log::info("%s\n", realType.c_str());

        aliasInfo["name"] = filterName(realType);
        aliasInfo["elementType"] = getTypeInfo(elementDataType);
        aliasInfo["trueType"] = getTypeInfo(trueDataType);

        setTemplateComments(aliasType, aliasInfo);

        aliases.push_back(aliasInfo);
        ++n;
        //        }
    }
    m_templateData["aliases"] = aliases;
}

data_map PythonGenerator::makeGroupSymbolsTemplateData(Group *group)
{
    data_map symbolsTemplate;
    set<string> names;

    data_list structs;
    data_list unions;
    data_list aliases;

    Log::info("Group symbols:\n");

    // generate templates for group symbols or for all symbols if group has no interface defined
    for (Symbol *symbol : (group->getInterfaces().empty() ? m_globals->getSymbolVector() : group->getSymbols()))
    {
        data_map info;

        if (symbol->isDatatypeSymbol())
        {
            DataType *dataType = dynamic_cast<DataType *>(symbol);
            assert(dataType);

            switch (dataType->getDataType())
            {
                case DataType::kStructType: {
                    StructType *structType = dynamic_cast<StructType *>(symbol);
                    if (structType == nullptr)
                    {
                        break;
                    }

                    Log::info("%s\n", structType->getDescription().c_str());

                    string name = filterName(getOutputName(structType));

                    // check if template for this structure has not already been generated
                    if (names.find(name) == names.end())
                    {
                        info["name"] = name;

                        setTemplateComments(structType, info);
                        setStructMembersTemplateData(structType, info);

                        names.insert(name);
                        structs.push_back(info);
                    }
                    break;
                }
                case DataType::kUnionType: {
                    UnionType *unionType = dynamic_cast<UnionType *>(symbol);
                    if (unionType == nullptr)
                    {
                        break;
                    }

                    Log::info("%s\n", unionType->getDescription().c_str());

                    string name = filterName(getOutputName(unionType));
                    if (name.find('$') != string::npos)
                    {
                        Log::debug("%s is inside struct!\n", name.c_str());
                        break;
                    }

                    // check if template for this structure has not already been generated
                    if (names.find(name) == names.end())
                    {
                        info["name"] = name;
                        info["type"] = getTypeInfo(unionType);

                        setTemplateComments(unionType, info);
                        // setUnionMembersTemplateData(unionType, info);

                        names.insert(name);
                        unions.push_back(info);
                    }
                    break;
                }
                case DataType::kAliasType: {
                    AliasType *aliasType = dynamic_cast<AliasType *>(symbol);
                    if (aliasType == nullptr)
                        break;

                    DataType *elementDataType = aliasType->getElementType();
                    DataType *trueDataType = elementDataType->getTrueDataType();
                    // Only generate aliases for enums, unions and structs in Python.
                    if (!(trueDataType->isEnum() || trueDataType->isUnion() || trueDataType->isStruct()))
                        break;

                    string realType = getOutputName(aliasType);
                    Log::debug("%s\n", realType.c_str());

                    info["name"] = filterName(realType);
                    info["elementType"] = getTypeInfo(elementDataType);
                    info["trueType"] = getTypeInfo(trueDataType);

                    setTemplateComments(aliasType, info);

                    aliases.push_back(info);
                    break;
                }
                default:
                    break;
            }
        }
    }

    symbolsTemplate["structs"] = structs;
    symbolsTemplate["unions"] = unions;
    symbolsTemplate["aliases"] = aliases;

    return symbolsTemplate;
}

void PythonGenerator::setStructMembersTemplateData(StructType *structType, data_map &structInfo)
{
    data_list members;
    for (auto member : structType->getMembers())
    {
        data_map member_info;

        // Skip data serialization for variables placed as @length value for lists.
        // These prevent to serialized data twice.
        StructMember *referencedFrom =
            findParamReferencedFromAnn(structType->getMembers(), member->getName(), LENGTH_ANNOTATION);

        member_info["discriminatorForMember"] = "";
        if (referencedFrom)
        {
            member_info["lengthForMember"] = getOutputName(referencedFrom);
        }
        else
        {
            member_info["lengthForMember"] = "";
            // Skip data serialization for variables used as discriminator for unions.
            // These prevent to serialized data twice.
            referencedFrom = findParamReferencedFromUnion(structType->getMembers(), member->getName());
            if (referencedFrom)
            {
                member_info["discriminatorForMember"] = getOutputName(referencedFrom);
            }
        }

        member_info["serializedViaMember"] = (referencedFrom) ? getOutputName(referencedFrom) : "";
        setOneStructMemberTemplateData(member, member_info);
        members.push_back(member_info);
    }
    structInfo["members"] = members;
}

void PythonGenerator::setOneStructMemberTemplateData(StructMember *member, data_map &member_info)
{
    string memberName = getOutputName(member);

    DataType *trueDataType = member->getDataType()->getTrueDataType();

    // Info for declaring struct in common header
    member_info["name"] = memberName;
    bool isNullable = ((findAnnotation(member, NULLABLE_ANNOTATION) != nullptr) &&
                       (trueDataType->isBinary() || trueDataType->isString() || trueDataType->isList()));
    member_info["isNullable"] = isNullable;
    member_info["type"] = getTypeInfo(member->getDataType());
    /* Necessary for handling non-discriminated unions */
    member_info["discriminator"] = getAnnStringValue(member, DISCRIMINATOR_ANNOTATION);

    setTemplateComments(member, member_info);
}

void PythonGenerator::makeFunctionsTemplateData()
{
    /* type definitions of functions and table of functions */
    Log::info("Functions:\n");
    data_list functions;
    for (Symbol *functionTypeSymbol : getDataTypesFromSymbolScope(m_globals, DataType::kFunctionType))
    {
        FunctionType *functionType = dynamic_cast<FunctionType *>(functionTypeSymbol);
        data_map functionInfo;

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
}

data_map PythonGenerator::getTypeInfo(DataType *t)
{
    data_map info;
    info["name"] = filterName(getOutputName(t, false));
    info["isNonEncapsulatedUnion"] = false;
    switch (t->getDataType())
    {
        case DataType::kAliasType: {
            info = getTypeInfo(t->getTrueDataType());
            break;
        }
        case DataType::kArrayType: {
            // Array type requires the array element count to come after the variable/member name.
            ArrayType *a = dynamic_cast<ArrayType *>(t);
            assert(a);
            info["type"] = "array";
            info["elementCount"] = a->getElementCount();
            info["elementType"] = getTypeInfo(a->getElementType());
            break;
        }
        case DataType::kBuiltinType: {
            assert(dynamic_cast<const BuiltinType *>(t));
            info["type"] = getBuiltinTypename(dynamic_cast<const BuiltinType *>(t));
            break;
        }
        case DataType::kEnumType: {
            info["type"] = "enum";
            break;
        }
        case DataType::kFunctionType: {
            info["type"] = "function";
            FunctionType *funType = dynamic_cast<FunctionType *>(t);
            assert(funType);
            const FunctionType::c_function_list_t &callbacks = funType->getCallbackFuns();
            if (callbacks.size() > 1)
            {
                info["tableName"] = "_" + funType->getName();
            }
            else if (callbacks.size() == 1)
            {
                info["tableName"] = "";
                info["callbackName"] = callbacks[0]->getName();
            }
            else
            {
                throw semantic_error(format_string("Function has function type parameter (callback parameter), but in "
                                                   "IDL is missing function definition, which can be passed there.")
                                         .c_str());
            }
            break;
        }
        case DataType::kListType: {
            const ListType *a = dynamic_cast<const ListType *>(t);
            assert(a);
            info["type"] = "list";
            info["elementType"] = getTypeInfo(a->getElementType());
            break;
        }
        case DataType::kStructType: {
            info["type"] = "struct";
            break;
        }
        case DataType::kUnionType: {
            UnionType *unionType = dynamic_cast<UnionType *>(t);
            assert(unionType);
            info["type"] = "union";

            // Different request for encapsulated and nonencapsulated unions
            if (unionType->isNonEncapsulatedUnion())
            {
                info["isNonEncapsulatedUnion"] = true;
                info["discriminatorName"] = "discriminator";
            }
            else
            {
                // Set discriminator field name.
                string discriminatorName = unionType->getDiscriminatorName();
                info["discriminatorName"] = discriminatorName;

                // Fill in discriminator type info.
                Symbol *discriminatorSym = unionType->getParentStruct()->getScope().getSymbol(discriminatorName);
                if (!discriminatorSym)
                {
                    throw semantic_error(
                        format_string("unable to find union discriminator '%s' in struct", discriminatorName.c_str()));
                }
                StructMember *discriminatorMember = dynamic_cast<StructMember *>(discriminatorSym);
                if (!discriminatorMember)
                {
                    throw internal_error(format_string("union discriminator is not a struct member"));
                }
                info["discriminatorType"] = getTypeInfo(discriminatorMember->getDataType());
            }

            data_list unionCases;
            for (auto unionCase : unionType->getCases())
            {
                data_map caseData;
                caseData["name"] = unionCase->getCaseName();
                caseData["value"] = unionCase->getCaseValue();
                if (info.has("discriminatorType") &&
                    (info["discriminatorType"].get().get()->getmap()["type"]->getvalue() == "enum"))
                {
                    caseData["type"] = info["discriminatorType"];
                }
                else if (unionCase->getCaseName() != "")
                {
                    for (auto it : getDataTypesFromSymbolScope(m_globals, DataType::kEnumType))
                    {
                        EnumType *enumType = dynamic_cast<EnumType *>(it);
                        assert(enumType);
                        for (auto itMember : enumType->getMembers())
                        {
                            if (unionCase->getCaseName() == itMember->getName())
                            {
                                data_map typeInfo;
                                typeInfo["name"] = enumType->getName();
                                typeInfo["type"] = "enum";
                                caseData["type"] = typeInfo;
                                break;
                            }
                        }
                        if (caseData.has("type"))
                        {
                            break;
                        }
                    }
                    if (!caseData.has("type"))
                    {
                        for (auto it : m_globals->getSymbolsOfType(DataType::kConstSymbol))
                        {
                            ConstType *constType = dynamic_cast<ConstType *>(it);
                            assert(constType);
                            if (unionCase->getCaseName() == constType->getName())
                            {
                                data_map typeInfo;
                                typeInfo["name"] = "";
                                typeInfo["type"] = "const";
                                caseData["type"] = typeInfo;
                                break;
                            }
                        }
                    }
                }
                else
                {
                    data_map typeInfo;
                    typeInfo["name"] = "";
                    typeInfo["type"] = "";
                    caseData["type"] = typeInfo;
                }
                data_list caseMembers;
                data_map caseMembersFree;

                if (unionCase->caseMemberIsVoid())
                {
                    caseData["isVoid"] = true;
                    caseData["members"] = data_list();
                }
                else
                {
                    caseData["isVoid"] = false;

                    // Fill in info for each case member declaration
                    for (auto caseMemberName : unionCase->getMemberDeclarationNames())
                    {
                        data_map memberData;
                        StructMember *member = unionCase->getUnionMemberDeclaration(caseMemberName);
                        setOneStructMemberTemplateData(member, memberData);

                        caseMembers.push_back(memberData);
                    }

                    caseData["members"] = caseMembers;
                }

                unionCases.push_back(caseData);
            }
            info["cases"] = unionCases;
            break;
        }
        case DataType::kVoidType: {
            info["type"] = "void";
            break;
        }
        default:
            throw internal_error("unknown data type");
    }
    return info;
}

string PythonGenerator::getBuiltinTypename(const BuiltinType *t)
{
    switch (t->getBuiltinType())
    {
        case BuiltinType::kBoolType:
            return "bool";
        case BuiltinType::kInt8Type:
            return "int8";
        case BuiltinType::kInt16Type:
            return "int16";
        case BuiltinType::kInt32Type:
            return "int32";
        case BuiltinType::kInt64Type:
            return "int64";
        case BuiltinType::kUInt8Type:
            return "uint8";
        case BuiltinType::kUInt16Type:
            return "uint16";
        case BuiltinType::kUInt32Type:
            return "uint32";
        case BuiltinType::kUInt64Type:
            return "uint64";
        case BuiltinType::kFloatType:
            return "float";
        case BuiltinType::kDoubleType:
            return "double";
        case BuiltinType::kStringType:
            return "string";
        case BuiltinType::kBinaryType:
            return "binary";
        default:
            throw internal_error("unknown builtin type");
    }
}

string PythonGenerator::filterName(const string &name)
{
    string result = name;
    if (result.size() >= m_suffixStripSize)
    {
        size_t pos = result.size() - m_suffixStripSize;
        if (result.substr(pos, m_suffixStripSize) == m_suffixStrip)
        {
            result.erase(pos, m_suffixStripSize);
        }
    }
    return result;
}

string PythonGenerator::convertComment(const string &comment, comment_type commentType)
{
    (void)commentType;
    // Longer patterns are ordered earlier than similar shorter patterns.
    static const char *const kCommentBegins[] = { "//!<", "//!", "///<", "///", "/*!<", "/*!", "/**<", "/**", 0 };
    static const char *const kCommentEnds[] = { "*/", 0 };

    string result = stripWhitespace(comment);
    int32_t i;
    uint32_t n;

    if (result.empty())
    {
        return result;
    }

    // Search for a matching comment begin to strip.
    for (i = 0; kCommentBegins[i] != 0; ++i)
    {
        size_t p = result.find(kCommentBegins[i]);
        if (p != string::npos)
        {
            // Remove the comment begin from the result.
            size_t l = strlen(kCommentBegins[i]);
            result = result.substr(p + l, result.size() - (p + l));
            break;
        }
    }

    // Check if we failed to find a matching comment begin.
    if (kCommentBegins[i] == 0)
    {
        throw internal_error("unable to convert Doxygen comment");
    }

    // Search for a matching comment end to strip. There may not be a comment end.
    for (i = 0; kCommentEnds[i] != 0; ++i)
    {
        size_t p = result.rfind(kCommentEnds[i]);
        if (p != string::npos)
        {
            // Remove the comment end from the result.
            result = result.substr(0, p);
            break;
        }
    }

    result = stripWhitespace(result);

    // Handle multiline comments.
    size_t p = 0;
    while (p < result.size())
    {
        // Erase over any whitespace, except newlines.
        for (n = 0; p + n < result.size(); ++n)
        {
            char c = result[p + n];
            if (c != ' ' && c != '\t')
            {
                break;
            }
        }
        if (n > 0)
        {
            result.erase(p, n);
        }

        if (p >= result.size())
        {
            break;
        }

        // Look for comment line headers.
        if (result[p] == '/' && result[p + 1] == '/' && result[p + 2] == '!')
        {
            result.erase(p, 3);
        }
        else if (result[p] == '/' && result[p + 1] == '/')
        {
            result.erase(p, 2);
        }
        else if (result[p] == '*')
        {
            result.erase(p, 1);
        }

        // Insert Python line comment start.
        result.insert(p, "#");

        // Find the next newline.
        p = result.find('\n', p + 1);
        if (p == string::npos)
        {
            break;
        }
        // Skip over the newline that we want to keep.
        ++p;
    }

    // Return comment converted to Python form.
    return result;
}

bool PythonGenerator::checkWhitspaceChar(char c)
{
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
    {
        return true;
    }
    return false;
}

string PythonGenerator::stripWhitespace(const string &s)
{
    string result = s;
    int32_t i;
    uint32_t n;

    // Strip leading whitespace.
    for (n = 0, i = 0; i < (int)result.size(); ++i, ++n)
    {
        char c = result[i];

        if ((i < (int)result.size() - 1 && c == ' ' && !checkWhitspaceChar(result[i + 1])) || !checkWhitspaceChar(c))
        {
            break;
        }
    }
    if (n > 0)
    {
        result.erase(0, n);
    }

    // Strip trailing whitespace.
    for (n = 0, i = (int)result.size() - 1; i > 0; --i, ++n)
    {
        char c = result[i];
        if (!checkWhitspaceChar(c))
        {
            break;
        }
    }
    if (n > 0)
    {
        result.erase(i + 1, n);
    }

    return result;
}

void PythonGenerator::initPythonReservedWords()
{
    // Python reserved words
    reserverdWords.insert("and");
    reserverdWords.insert("assert");
    reserverdWords.insert("break");
    reserverdWords.insert("class");
    reserverdWords.insert("continue");
    reserverdWords.insert("def");
    reserverdWords.insert("del");
    reserverdWords.insert("elif");
    reserverdWords.insert("else");
    reserverdWords.insert("except");
    reserverdWords.insert("exec");
    reserverdWords.insert("finally");
    reserverdWords.insert("for");
    reserverdWords.insert("from");
    reserverdWords.insert("global");
    reserverdWords.insert("if");
    reserverdWords.insert("import");
    reserverdWords.insert("in");
    reserverdWords.insert("is");
    reserverdWords.insert("lambda");
    reserverdWords.insert("not");
    reserverdWords.insert("or");
    reserverdWords.insert("pass");
    reserverdWords.insert("print");
    reserverdWords.insert("raise");
    reserverdWords.insert("return");
    reserverdWords.insert("try");
    reserverdWords.insert("while");
}
