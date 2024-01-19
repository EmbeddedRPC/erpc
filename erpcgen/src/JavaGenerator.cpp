/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "JavaGenerator.hpp"

#include "Logging.hpp"
#include "ParseErrors.hpp"
#include "annotations.h"
#include "format_string.hpp"

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <set>
#include <sstream>

using namespace erpcgen;
using namespace cpptempl;
using namespace std;

// Templates strings converted from text files by txt_to_c.py.
extern const char *const kJavaCoders;
extern const char *const kJavaEnum;
extern const char *const kJavaStruct;
extern const char *const kJavaServer;
extern const char *const kJavaClient;
extern const char *const kJavaConst;
extern const char *const kJavaInterface;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

JavaGenerator::JavaGenerator(InterfaceDefinition *def, const std::string javaPackageName)
: Generator(def, generator_type_t::kJava)
, m_javaPackageName(javaPackageName)
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

void JavaGenerator::generateOutputFiles(const string &fileName)
{
    // Make sure the package folder is created.
    filesystem::path dir(fileName);
    dir = m_outputDirectory / dir;
    filesystem::create_directories(dir);

    generateCommonFile(fileName);
    generateClientFile(fileName);
    generateServerFile(fileName);
    generateInterfaceFile(fileName);
}

void JavaGenerator::generateCommonFile(string fileName)
{
    generateEnumFiles(fileName);
    generateStructFiles(fileName);
    generateConstFile(fileName);
}

void JavaGenerator::generateConstFile(string fileName)
{
    fileName += "/common/";

    filesystem::path dir(fileName);
    dir = m_outputDirectory / dir;
    filesystem::create_directories(dir);

    std::string classFilename = fileName + "/Constants.java";
    generateOutputFile(classFilename, "java_const", m_templateData, kJavaConst);
}

void JavaGenerator::generateClientFile(string fileName)
{
    data_map &map = m_templateData["group"].get().get()->getmap();
    data_list &list = map["interfaces"]->getlist();
    fileName += "/client";

    filesystem::path dir(fileName);
    dir = m_outputDirectory / dir;
    filesystem::create_directories(dir);

    for (data_ptr interface : list)
    {
        std::string name = interface.get().get()->getmap()["name"].get().get()->getvalue();
        m_templateData["interface"] = interface;
        std::string classFilename = fileName + "/" + name + "Client.java";
        generateOutputFile(classFilename, "java_client", m_templateData, kJavaClient);
    }
}

void JavaGenerator::generateServerFile(string fileName)
{
    data_map &map = m_templateData["group"].get().get()->getmap();
    data_list &list = map["interfaces"]->getlist();
    fileName += "/server";

    filesystem::path dir(fileName);
    dir = m_outputDirectory / dir;
    filesystem::create_directories(dir);

    for (data_ptr interface : list)
    {
        std::string name = interface.get().get()->getmap()["name"].get().get()->getvalue();
        m_templateData["interface"] = interface;
        std::string classFilename = fileName + "/Abstract" + name + "Service.java";
        generateOutputFile(classFilename, "java_server", m_templateData, kJavaServer);
    }
}

void JavaGenerator::generateInterfaceFile(string fileName)
{
    data_map &map = m_templateData["group"].get().get()->getmap();
    data_list &list = map["interfaces"]->getlist();
    fileName += "/interfaces";

    filesystem::path dir(fileName);
    dir = m_outputDirectory / dir;
    filesystem::create_directories(dir);

    for (data_ptr interface : list)
    {
        std::string name = interface.get().get()->getmap()["name"].get().get()->getvalue();
        m_templateData["interface"] = interface;
        std::string classFilename = fileName + "/I" + name + ".java";
        generateOutputFile(classFilename, "java_interface", m_templateData, kJavaInterface);
    }
}

void erpcgen::JavaGenerator::generateEnumFiles(std::string fileName)
{
    data_list &list = m_templateData["enums"]->getlist();
    fileName += "/common/enums";

    filesystem::path dir(fileName);
    dir = m_outputDirectory / dir;
    filesystem::create_directories(dir);

    for (data_ptr item : list)
    {
        std::string name = item.get().get()->getmap()["name"].get().get()->getvalue();

        if (name.empty())
        {
            name = "ConstantEnum"; // TODO: Create in ./common
            item.get().get()->getmap()["name"] = make_data("ConstantEnum");
        }

        m_templateData["enum"] = item;
        std::string classFilename = fileName + "/" + name + ".java";
        generateOutputFile(classFilename, "java_enum", m_templateData, kJavaEnum);
    }
}

void erpcgen::JavaGenerator::generateStructFiles(std::string fileName)
{
    data_list &list = m_templateData.parse_path("group.symbolsMap.structs").get().get()->getlist();
    fileName += "/common/structs";

    filesystem::path dir(fileName);
    dir = m_outputDirectory / dir;
    filesystem::create_directories(dir);

    for (data_ptr item : list)
    {
        std::string name = item.get().get()->getmap()["name"].get().get()->getvalue();

        m_templateData["struct"] = item;
        // dump_data(item);
        std::string classFilename = fileName + "/" + name + ".java";
        generateOutputFile(classFilename, "java_struct", m_templateData, kJavaStruct);
    }
}

void JavaGenerator::parseSubtemplates()
{
    const char *templateName = "java_coders";
    try
    {
        parse(kJavaCoders, m_templateData);
    }
    catch (TemplateException &e)
    {
        throw TemplateException(format_string("Template %s: %s", templateName, e.what()));
    }
}

void JavaGenerator::generate()
{
    data_list empty;
    m_templateData["enums"] = empty;
    m_templateData["aliases"] = empty;
    m_templateData["structs"] = empty;
    m_templateData["unions"] = empty;
    m_templateData["consts"] = empty;
    m_templateData["appPackage"] = make_data(m_javaPackageName);

    parseSubtemplates();

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
        m_templateData["groupPackage"] = getGroupPackageName(group);
        generateGroupOutputFiles(group);
    }
}

void JavaGenerator::setTemplateComments(Symbol *symbol, data_map &symbolInfo)
{
    symbolInfo["mlComment"] = symbol->getMlComment();
    symbolInfo["ilComment"] = symbol->getIlComment();
}

data_map JavaGenerator::getFunctionTemplateData(Group *group, Function *fn)
{
    (void)group;
    data_map info;
    string proto = getFunctionPrototype(nullptr, fn);

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
    DataType *returnDataType = fn->getReturnType()->getTrueDataType();
    returnInfo["type"] = getTypeInfo(returnDataType, false);
    if (!returnDataType->isVoid())
    {
        returnInfo["call"] = getEncodeDecodeCall("_result", returnDataType, nullptr, false, false,
                                                 fn->getReturnStructMemberType(), true, false);
    }
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
        paramInfo["type"] = getTypeInfo(paramType, false);

        bool isNullable = ((findAnnotation(param, NULLABLE_ANNOTATION) != nullptr) &&
                           (trueDataType->isString() || (!trueDataType->isBuiltin() && !trueDataType->isEnum())));
        paramInfo["isNullable"] = isNullable;

        paramInfo["call"] = getEncodeDecodeCall(name, trueDataType, nullptr, false, false, param, false, true);

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

        param_direction_t dir = param->getDirection();
        switch (dir)
        {
            case param_direction_t::kInDirection:
                paramInfo["direction"] = "in";
                inParams.push_back(paramInfo);
                break;
            case param_direction_t::kOutDirection:
                paramInfo["direction"] = "out";
                outParams.push_back(paramInfo);
                break;
            case param_direction_t::kInoutDirection:
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

string JavaGenerator::getFunctionPrototype(Group *group, FunctionBase *fn, const string &interfaceName,
                                             const string &name, bool insideInterfaceCall)
{
    FunctionType *functionType = dynamic_cast<FunctionType *>(fn);
    if (functionType)
    {
        return ""; /*Todo: implement*/
    }
    Function *function = dynamic_cast<Function *>(fn);

    assert(function);

    DataType *dataTypeReturn = fn->getReturnType();
    string proto;

    proto = getTypenameName(dataTypeReturn, false, false) + " ";
    proto += getOutputName(function);

    bool isFirst = true;

    proto += "(";

    auto params = function->getParameters().getMembers();
    if (params.size())
    {
        for (auto it : params)
        {
            string paramSignature = getOutputName(it);

            // Skip data serialization for variables placed as @length value for lists.
            if (findParamReferencedFromAnn(params, getOutputName(it), LENGTH_ANNOTATION))
            {
               continue;
            }

            if (!isFirst)
            {
               proto += ", ";
               
            }
            else
            {
               isFirst = false;
            }

            if (it->getDirection() == param_direction_t::kOutDirection || it->getDirection() == param_direction_t::kInoutDirection)
            {
                proto += getTypenameName(it->getDataType(), true, true);
            }
            else
            {
                proto += getTypenameName(it->getDataType(), false, false);
            }

            proto += " ";
            proto += getOutputName(it);
        }
    }
    proto += ")";
    return proto;
}

string JavaGenerator::getTypenameName(DataType *t, bool isReference, bool objectType)
{
    string returnName;



    switch (t->getDataType())
    {
        case DataType::data_type_t::kArrayType: {
            ArrayType *a = dynamic_cast<ArrayType *>(t);
            assert(a);
            returnName = getTypenameName(a->getElementType(), false, false) + "[]";
            break;
        }
        case DataType::data_type_t::kBuiltinType: {
            assert(nullptr != dynamic_cast<const BuiltinType *>(t));
            if (objectType)
            {
                returnName = getBuiltinObjectTypename(dynamic_cast<const BuiltinType *>(t));
            }
            else
            {
                returnName = getBuiltinTypename(dynamic_cast<const BuiltinType *>(t));
            }

            break;
        }
        case DataType::data_type_t::kListType: {
            const ListType *a = dynamic_cast<const ListType *>(t);
            assert(a);
            returnName = "List<" + getTypenameName(a->getElementType(), false, true) + ">";
            break;
        }
        case DataType::data_type_t::kAliasType: {
            AliasType *aliasType = dynamic_cast<AliasType *>(t);
            return getTypenameName(aliasType->getTrueDataType(), isReference, objectType);
        }
        case DataType::data_type_t::kVoidType: {
            returnName = "void";
            break;
        }
        case DataType::data_type_t::kUnionType:
        case DataType::data_type_t::kEnumType:
        case DataType::data_type_t::kStructType: {
            returnName = getOutputName(t);
            break;
        }
        default:
            throw internal_error(format_string("In getTypenameName: unknown data type: %s value:%d",
                                               t->getName().c_str(), t->getDataType()));
    }

    if (isReference)
    {
        returnName = "Reference<" + returnName + ">";
    }
    return returnName;
}

string JavaGenerator::getArrayInitialization(ArrayType *t)
{
    string result = "";

    DataType *current = dynamic_cast<DataType *>(t);
    while (current->isArray())
    {
        ArrayType *arrayType = dynamic_cast<ArrayType *>(current);
        result += format_string("[%d]", arrayType->getElementCount());
        current = arrayType->getElementType();
    }

    if (current->getDataType() == DataType::data_type_t::kListType)
    {
        result = "List" + result; // Java does not support typed arrays e.g. List<int[]>. Use untyped List[].
    }
    else
    {
        result = getTypenameName(t->getTrueContainerDataType(), false, false) + result;
    }

    result = "new " + result;

    return result;
}

void JavaGenerator::makeConstTemplateData()
{
    Log::info("Constant globals:\n");
    data_list consts;
    for (auto it : m_globals->getSymbolsOfType(Symbol::symbol_type_t::kConstSymbol))
    {
        ConstType *constVar = dynamic_cast<ConstType *>(it);
        assert(constVar);
        data_map constInfo;
        if (!findAnnotation(constVar, EXTERNAL_ANNOTATION))
        {
            constInfo["name"] = getOutputName(constVar);
            constInfo["type"] = getTypeInfo(constVar->getDataType(), false, false);
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

void JavaGenerator::makeEnumsTemplateData()
{
    Log::info("Enums:\n");
    data_list enums;
    int n = 0;
    for (auto it : getDataTypesFromSymbolScope(m_globals, DataType::data_type_t::kEnumType))
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

data_list JavaGenerator::getEnumMembersTemplateData(EnumType *enumType)
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

void JavaGenerator::makeAliasesTemplateData()
{
    Log::info("Type definition:\n");
    data_list aliases;
    int n = 0;
    for (auto it : getDataTypesFromSymbolScope(m_globals, DataType::data_type_t::kAliasType))
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
        aliasInfo["elementType"] = getTypeInfo(elementDataType, false);
        aliasInfo["trueType"] = getTypeInfo(trueDataType, false);

        setTemplateComments(aliasType, aliasInfo);

        aliases.push_back(aliasInfo);
        ++n;
        //        }
    }
    m_templateData["aliases"] = aliases;
}

data_map JavaGenerator::makeGroupSymbolsTemplateData(Group *group)
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
                case DataType::data_type_t::kStructType: {
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
                case DataType::data_type_t::kUnionType: {
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
                        info["type"] = getTypeInfo(unionType, false);

                        setTemplateComments(unionType, info);
                        // setUnionMembersTemplateData(unionType, info);

                        names.insert(name);
                        unions.push_back(info);
                    }
                    break;
                }
                case DataType::data_type_t::kAliasType: {
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
                    info["elementType"] = getTypeInfo(elementDataType, false);
                    info["trueType"] = getTypeInfo(trueDataType, false);

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

void JavaGenerator::setStructMembersTemplateData(StructType *structType, data_map &structInfo)
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

void JavaGenerator::setOneStructMemberTemplateData(StructMember *member, data_map &member_info)
{
    string memberName = getOutputName(member);
    bool needTempVariable = false;

    DataType *trueDataType = member->getDataType()->getTrueDataType();

    // Info for declaring struct in common header
    member_info["name"] = memberName;
    bool isNullable = ((findAnnotation(member, NULLABLE_ANNOTATION) != nullptr) &&
                       (trueDataType->isBinary() || trueDataType->isString() || trueDataType->isList()));
    member_info["isNullable"] = isNullable;
    member_info["call"] =
        getEncodeDecodeCall(memberName, trueDataType, nullptr, true, true, member, needTempVariable, false);
    // member_info["type"] = getTypeInfo(member->getDataType());
    /* Necessary for handling non-discriminated unions */
    member_info["discriminator"] = getAnnStringValue(member, DISCRIMINATOR_ANNOTATION);

    setTemplateComments(member, member_info);
}

data_map JavaGenerator::getEncodeDecodeCall(const string &name, DataType *t, StructType *structType,
                                            bool inDataContainer, bool isStructMember, StructMember *structMember,
                                            bool needTypeDeclaration, bool isFunctionParam)
{
    static uint8_t listArrayCounter; // Used for creating nested loops variable names
    data_map templateData;
    bool isReference = (isStructMember && structMember->isByref()) || 
                       (isFunctionParam && (structMember->getDirection() == param_direction_t::kOutDirection ||
                                            structMember->getDirection() == param_direction_t::kInoutDirection));
    templateData["type"] = getTypeInfo(t, isReference); // Type info about variable
    templateData["inDataContainer"] = inDataContainer;
    templateData["name"] = name;                               // Name of the variable
    templateData["isStructMember"] = isStructMember;           // If variable is part of struct
    templateData["needTypeDeclaration"] = needTypeDeclaration; // Variable does not exist in the scope
    templateData["isFunctionParam"] = isFunctionParam;
    templateData["isReference"] = isReference;

    switch (t->getDataType())
    {
        case DataType::data_type_t::kAliasType: {
            AliasType *aliasType = dynamic_cast<AliasType *>(t);
            assert(aliasType);
            templateData = getEncodeDecodeCall(name, aliasType->getElementType(), structType, inDataContainer, false,
                                               structMember, true, false);
            break;
        }
        case DataType::data_type_t::kArrayType: {
            ArrayType *arrayType = dynamic_cast<ArrayType *>(t);
            assert(arrayType);
            DataType *elementType = arrayType->getElementType()->getTrueDataType();

            string arrayName = name;
            templateData["decode"] = m_templateData["decodeArrayType"];
            templateData["encode"] = m_templateData["encodeArrayType"];

            templateData["initialization"] = getArrayInitialization(arrayType);

            listArrayCounter++;
            templateData["counter"] = format_string("genLoopVariable%d", listArrayCounter);
            templateData["protoNext"] =
                getEncodeDecodeCall(format_string("genValueVariable%d", listArrayCounter), elementType, structType,
                                    true, false, structMember, true, false);
            listArrayCounter--;

            templateData["size"] = format_string("%d", arrayType->getElementCount());
            templateData["isElementArrayType"] = elementType->isArray();
            break;
        }
        case DataType::data_type_t::kBuiltinType: {
            templateData["decode"] = m_templateData["decodeBuiltinType"];
            templateData["encode"] = m_templateData["encodeBuiltinType"];
            break;
        }
        case DataType::data_type_t::kEnumType: {
            templateData["decode"] = m_templateData["decodeEnumType"];
            templateData["encode"] = m_templateData["encodeEnumType"];
            break;
        }
        case DataType::data_type_t::kFunctionType: {
            throw internal_error("Java does not support functions yet.");
        }
        case DataType::data_type_t::kListType: {
            ListType *listType = dynamic_cast<ListType *>(t);
            assert(listType);
            DataType *elementType = listType->getElementType()->getTrueDataType();

            templateData["decode"] = m_templateData["decodeListType"];
            templateData["encode"] = m_templateData["encodeListType"];

            string nextName;

            listArrayCounter++;
            templateData["counter"] = format_string("getLoopVariable%d", listArrayCounter);
            templateData["protoNext"] =
                getEncodeDecodeCall(format_string("genValueVariable%d", listArrayCounter), elementType, structType,
                                    true, false, structMember, true, false);
            listArrayCounter--;

            if (listType->hasLengthVariable())
            {
                templateData["hasLengthVariable"] = true;
                Symbol *symbol = m_globals->getSymbol(listType->getLengthVariableName());
                if (symbol)
                {
                    ConstType *constType = dynamic_cast<ConstType *>(symbol);
                    assert(constType);
                    templateData["size"] = constType->getValue()->toString();
                }
                else
                {
                    templateData["size"] = listType->getLengthVariableName();
                }
            }
            else
            {
                templateData["hasLengthVariable"] = false;
            }

            templateData["sizeVariable"] =
                isStructMember || isFunctionParam ?
                    format_string("i%s", capitalize(name).c_str()) :
                    format_string("genSizeVariable%d", listArrayCounter); // TODO: Const or size variable name

            break;
        }
        case DataType::data_type_t::kStructType: {
            templateData["decode"] = m_templateData["decodeStructType"];
            templateData["encode"] = m_templateData["encodeStructType"];
            break;
        }
        case DataType::data_type_t::kUnionType: {
            break;
        }
        default: {
            throw internal_error("unknown member type");
        }
    }

    return templateData;
}

string JavaGenerator::capitalize(string text)
{
    text[0] = toupper(text[0]);
    return text;
}

void JavaGenerator::makeFunctionsTemplateData()
{
    /* type definitions of functions and table of functions */
    Log::info("Functions:\n");
    data_list functions;
    for (Symbol *functionTypeSymbol : getDataTypesFromSymbolScope(m_globals, DataType::data_type_t::kFunctionType))
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
        functionInfo["name"] = functionType->getName();
        /* Function type name. */
        functions.push_back(functionInfo);
    }
    m_templateData["functions"] = functions;
}

data_map JavaGenerator::getTypeInfo(DataType *t, int isReference, bool inDataContainer)
{
    data_map info;
    std::string name = filterName(getOutputName(t, false));
    info["name"] = name;
    info["typeName"] = getTypenameName(t, isReference, isReference); // Java type: Float, List<int[]>, ...

    info["isNonEncapsulatedUnion"] = false;
    switch (t->getDataType())
    {
        case DataType::data_type_t::kAliasType: {
            info = getTypeInfo(t->getTrueDataType(), false);
            break;
        }
        case DataType::data_type_t::kArrayType: {
            // Array type requires the array element count to come after the variable/member name.
            info["type"] = "array";
            break;
        }
        case DataType::data_type_t::kBuiltinType: {
            assert(dynamic_cast<const BuiltinType *>(t));
            info["type"] = getBuiltinTypename(dynamic_cast<const BuiltinType *>(t));
            info["codecTypeName"] = getBuiltinCodecTypeName(dynamic_cast<const BuiltinType *>(t));
            break;
        }
        case DataType::data_type_t::kEnumType: {
            info["type"] = "enum";
            break;
        }
        case DataType::data_type_t::kFunctionType: {
            info["type"] = "function";
            break;
        }
        case DataType::data_type_t::kListType: {
            info["type"] = "list";
            break;
        }
        case DataType::data_type_t::kStructType: {
            info["type"] = "struct";
            info["decode"] = m_templateData["decodeStructType"];
            info["encode"] = m_templateData["encodeStructType"];
            break;
        }
        case DataType::data_type_t::kUnionType: {
            UnionType *unionType = dynamic_cast<UnionType *>(t);
            assert(unionType);
            info["type"] = "union";
            info["encode"] = m_templateData["decodeUnionType"];
            info["decode"] = m_templateData["encodeUnionType"];

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
                info["discriminatorType"] = getTypeInfo(discriminatorMember->getDataType(), false);
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
                    for (auto it : getDataTypesFromSymbolScope(m_globals, DataType::data_type_t::kEnumType))
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
                        for (auto it : m_globals->getSymbolsOfType(DataType::symbol_type_t::kConstSymbol))
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
        case DataType::data_type_t::kVoidType: {
            info["type"] = "void";
            break;
        }
        default:
            throw internal_error("unknown data type");
    }
    return info;
}

string JavaGenerator::getBuiltinCodecTypeName(const BuiltinType *t)
{
    switch (t->getBuiltinType())
    {
        case BuiltinType::builtin_type_t::kBoolType:
            return "Bool";
        case BuiltinType::builtin_type_t::kInt8Type:
            return "Int8";
        case BuiltinType::builtin_type_t::kInt16Type:
            return "Int16";
        case BuiltinType::builtin_type_t::kInt32Type:
            return "Int32";
        case BuiltinType::builtin_type_t::kInt64Type:
            return "Int64";
        case BuiltinType::builtin_type_t::kUInt8Type:
            return "UInt8";
        case BuiltinType::builtin_type_t::kUInt16Type:
            return "UInt16";
        case BuiltinType::builtin_type_t::kUInt32Type:
            return "UInt32";
        case BuiltinType::builtin_type_t::kUInt64Type:
            throw internal_error("Java implementation does not support uint64");
        case BuiltinType::builtin_type_t::kFloatType:
            return "Float";
        case BuiltinType::builtin_type_t::kDoubleType:
            return "Double";
        case BuiltinType::builtin_type_t::kStringType:
            return "String";
        case BuiltinType::builtin_type_t::kBinaryType:
            return "Binary";
        default:
            throw internal_error("unknown builtin type");
    }
}

string JavaGenerator::getBuiltinTypename(const BuiltinType *t)
{
    switch (t->getBuiltinType())
    {
        case BuiltinType::builtin_type_t::kBoolType:
            return "boolean";
        case BuiltinType::builtin_type_t::kInt8Type:
            return "byte";
        case BuiltinType::builtin_type_t::kInt16Type:
            return "short";
        case BuiltinType::builtin_type_t::kInt32Type:
            return "int";
        case BuiltinType::builtin_type_t::kInt64Type:
            return "long";
        case BuiltinType::builtin_type_t::kUInt8Type:
            return "short";
        case BuiltinType::builtin_type_t::kUInt16Type:
            return "int";
        case BuiltinType::builtin_type_t::kUInt32Type:
            return "long";
        case BuiltinType::builtin_type_t::kUInt64Type:
            throw internal_error("Java implementation does not support uint64");
        case BuiltinType::builtin_type_t::kFloatType:
            return "float";
        case BuiltinType::builtin_type_t::kDoubleType:
            return "double";
        case BuiltinType::builtin_type_t::kStringType:
            return "String";
        case BuiltinType::builtin_type_t::kBinaryType:
            return "byte[]";
        default:
            throw internal_error("unknown builtin type");
    }
}

string JavaGenerator::getBuiltinObjectTypename(const BuiltinType *t)
{
    switch (t->getBuiltinType())
    {
        case BuiltinType::builtin_type_t::kBoolType:
            return "Boolean";
        case BuiltinType::builtin_type_t::kInt8Type:
            return "Byte";
        case BuiltinType::builtin_type_t::kInt16Type:
            return "Short";
        case BuiltinType::builtin_type_t::kInt32Type:
            return "Integer";
        case BuiltinType::builtin_type_t::kInt64Type:
            return "Long";
        case BuiltinType::builtin_type_t::kUInt8Type:
            return "Short";
        case BuiltinType::builtin_type_t::kUInt16Type:
            return "Integer";
        case BuiltinType::builtin_type_t::kUInt32Type:
            return "Long";
        case BuiltinType::builtin_type_t::kUInt64Type:
            throw internal_error("Java implementation does not support uint64");
        case BuiltinType::builtin_type_t::kFloatType:
            return "Float";
        case BuiltinType::builtin_type_t::kDoubleType:
            return "Double";
        case BuiltinType::builtin_type_t::kStringType:
            return "String";
        case BuiltinType::builtin_type_t::kBinaryType:
            return "byte[]";
        default:
            throw internal_error("unknown builtin type");
    }
}

string JavaGenerator::filterName(const string &name)
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

std::string erpcgen::JavaGenerator::getGroupPackageName(const Group *group)
{
    string name = m_javaPackageName;

    if (m_def->hasProgramSymbol() && !m_def->getProgramName().empty())
    {
        name += "." + m_def->getProgramName();

        if (!group->getName().empty())
        {
            name += "_" + group->getName();
        }
    }
    else
    {
        if (!group->getName().empty())
        {
            name += "." + group->getName();
        }
    }

    return name;
}

bool JavaGenerator::checkWhitespaceChar(char c)
{
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
    {
        return true;
    }
    return false;
}

string JavaGenerator::stripWhitespace(const string &s)
{
    string result = s;
    int32_t i;
    uint32_t n;

    // Strip leading whitespace.
    for (n = 0, i = 0; i < (int)result.size(); ++i, ++n)
    {
        char c = result[i];

        if ((i < (int)result.size() - 1 && c == ' ' && !checkWhitespaceChar(result[i + 1])) || !checkWhitespaceChar(c))
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
        if (!checkWhitespaceChar(c))
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

void JavaGenerator::initJavaReservedWords()
{

    // Java reserved words
    reserverdWords.insert("abstract");
    reserverdWords.insert("continue");
    reserverdWords.insert("for");
    reserverdWords.insert("new");
    reserverdWords.insert("switch");
    reserverdWords.insert("assert");
    reserverdWords.insert("default");
    reserverdWords.insert("goto");
    reserverdWords.insert("package");
    reserverdWords.insert("synchronized");
    reserverdWords.insert("boolean");
    reserverdWords.insert("do");
    reserverdWords.insert("if");
    reserverdWords.insert("private");
    reserverdWords.insert("this");
    reserverdWords.insert("break");
    reserverdWords.insert("double");
    reserverdWords.insert("implements");
    reserverdWords.insert("protected");
    reserverdWords.insert("throw");
    reserverdWords.insert("byte");
    reserverdWords.insert("else");
    reserverdWords.insert("import");
    reserverdWords.insert("public");
    reserverdWords.insert("throws");
    reserverdWords.insert("case");
    reserverdWords.insert("enum");
    reserverdWords.insert("instanceof");
    reserverdWords.insert("return");
    reserverdWords.insert("transient");
    reserverdWords.insert("catch");
    reserverdWords.insert("extends");
    reserverdWords.insert("int");
    reserverdWords.insert("short");
    reserverdWords.insert("try");
    reserverdWords.insert("char");
    reserverdWords.insert("final");
    reserverdWords.insert("interface");
    reserverdWords.insert("static");
    reserverdWords.insert("void");
    reserverdWords.insert("class");
    reserverdWords.insert("finally");
    reserverdWords.insert("long");
    reserverdWords.insert("strictfp");
    reserverdWords.insert("volatile");
    reserverdWords.insert("const");
    reserverdWords.insert("float");
    reserverdWords.insert("native");
    reserverdWords.insert("super");
    reserverdWords.insert("while");
}
