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

#include "PythonGenerator.h"
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

// Templates strings converted from text files by txt_to_c.py.
extern const char *const kPyCoders;
extern const char *const kPyInit;
extern const char *const kPyCommon;
extern const char *const kPyServer;
extern const char *const kPyClient;
extern const char *const kPyInterface;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

PythonGenerator::PythonGenerator(InterfaceDefinition *def)
: Generator(def)
, m_suffixStrip("")
, m_suffixStripSize(0)
{
}

void PythonGenerator::generateOutputFiles(const std::string &fileName)
{
    boost::filesystem::path outputDir = m_def->getOutputDirectory();
    std::string commonFileName = stripExtension(m_def->getOutputFilename()) + fileName;

    // Make sure the package folder is created.
    boost::filesystem::path dir(commonFileName);
    dir = outputDir / dir;
    boost::filesystem::create_directories(dir);

    generateInitFile(commonFileName);
    generateCommonFile(commonFileName);
    generateClientFile(commonFileName);
    generateServerFile(commonFileName);
    generateInterfaceFile(commonFileName);
}

void PythonGenerator::generateInitFile(std::string fileName)
{
    fileName += "/__init__.py";
    generateOutputFile(fileName, "py_init", m_templateData, kPyInit);
};

void PythonGenerator::generateCommonFile(std::string fileName)
{
    fileName += "/common.py";
    generateOutputFile(fileName, "py_common", m_templateData, kPyCommon);
}

void PythonGenerator::generateClientFile(std::string fileName)
{
    fileName += "/client.py";
    generateOutputFile(fileName, "py_client", m_templateData, kPyClient);
}

void PythonGenerator::generateServerFile(std::string fileName)
{
    fileName += "/server.py";
    generateOutputFile(fileName, "py_server", m_templateData, kPyServer);
}

void PythonGenerator::generateInterfaceFile(std::string fileName)
{
    fileName += "/interface.py";
    generateOutputFile(fileName, "py_interface", m_templateData, kPyInterface);
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
    m_templateData["consts"] = empty;

    m_templateData["erpcgenVersion"] = ERPCGEN_VERSION;

    std::time_t now = std::time(nullptr);
    std::string nowString = std::ctime(&now);
    nowString.pop_back(); // Remove trailing newline.
    m_templateData["todaysDate"] = nowString;

    parseSubtemplates();

    if (m_def->hasProgramSymbol())
    {
        Log::info("program: ");
        Log::info("%s\n", m_def->getOutputFilename().c_str());

        for (auto anno: m_def->programSymbol()->getAnnotations(PY_TYPES_NAME_STRIP_SUFFIX_ANNOTATION))
        {
            m_suffixStrip = anno->getValueObject()->toString();
            m_suffixStripSize = m_suffixStrip.size();
        }
    }

    // makeIncludesTemplateData();

    makeInterfacesTemplateData();

    makeConstTemplateData();

    makeEnumsTemplateData();

    makeAliasesTemplateData();

    makeStructsTemplateData();

    // Log template data.
    if (Log::getLogger()->getFilterLevel() >= Logger::kDebug2)
    {
        dump_data(m_templateData);
    }

    generateOutputFiles("");
}

// void PythonGenerator::makeIncludesTemplateData()
//{
//    data_list includeData;
//    if(m_def->hasProgramSymbol())
//    {
//        for(auto include: m_def->programSymbol()->getAnnotations(INCLUDE_ANNOTATION))
//        {
//            includeData.push_back(make_data(include->getValueObject()->toString()));
//            Log::info("#include %s\n", include->getValueObject()->toString().c_str());
//        }
//    }
//    m_templateData["includes"] = includeData;
//}

void PythonGenerator::makeInterfacesTemplateData()
{
    Log::info("interfaces:\n");
    int n = 0;
    data_list ifaces;
    for (auto it : m_globals->getSymbolsOfType(Symbol::kInterfaceSymbol))
    {
        Interface *iface = dynamic_cast<Interface *>(it);
        assert(iface);

        data_map ifaceInfo;
        ifaceInfo["name"] = make_data(iface->getName());
        ifaceInfo["id"] = data_ptr(iface->getUniqueId());
        ifaceInfo["mlComment"] = convertComment(iface->getMlComment(), kMultilineComment);
        ifaceInfo["ilComment"] = convertComment(iface->getIlComment(), kInlineComment);

        Log::info("%d: (%d) %s\n", n, iface->getUniqueId(), iface->getName().c_str());

        ifaceInfo["functions"] = getFunctionsTemplateData(iface);
        ifaces.push_back(make_data(ifaceInfo));
        ++n;
    }
    m_templateData["interfaces"] = ifaces;
}

data_list PythonGenerator::getFunctionsTemplateData(Interface *iface)
{
    data_list fns;
    int j = 0;
    for (auto fit : iface->getFunctions())
    {
        fns.push_back(getFunctionTemplateData(fit, j++));
    }
    return fns;
}

//! TODO This should be common code.
void PythonGenerator::scanStructForLengthAnnotation(StructType *theStruct)
{
    SymbolScope &structScope = theStruct->getScope();
    for (auto member : theStruct->getMembers())
    {
        DataType *memberType = member->getDataType();
        DataType *memberTrueType = memberType->getTrueDataType();

        // Look for @length annotation and skip this member if not present.
        Annotation *lengthAnno = member->findAnnotation(LENGTH_ANNOTATION);
        if (!lengthAnno)
        {
            continue;
        }

        // Check the type of the member with the @length annotation.
        if (!(memberTrueType->isList() || memberTrueType->isBinary() || memberTrueType->isString()))
        {
            throw semantic_error(format_string("line %d: Length annotation can only be applied to list, binary, or string types", lengthAnno->getLocation().m_firstLine));
        }

        // Check @length annotation's value.
        if (!lengthAnno->hasValue())
        {
            throw semantic_error(format_string("line %d: Length annotation must name a valid parameter or member", lengthAnno->getLocation().m_firstLine));
        }

        // Make sure the @length annotation names a valid member.
        std::string lengthIdent = lengthAnno->getValueObject()->toString();
        if (!structScope.hasSymbol(lengthIdent))
        {
            throw semantic_error(format_string("line %d: Length annotation must name a valid parameter or member", lengthAnno->getLocation().m_firstLine));
        }

        Symbol *sym = structScope.getSymbol(lengthIdent);
        StructMember *lengthMember = dynamic_cast<StructMember*>(sym);
        assert(lengthMember);

        // Verify the length member is a scalar.
        DataType *lengthMemberType = lengthMember->getDataType()->getTrueDataType();
        BuiltinType *lengthMemberBuiltinType = dynamic_cast<BuiltinType*>(lengthMemberType);
        if (!lengthMemberBuiltinType || !lengthMemberBuiltinType->isInt())
        {
            throw semantic_error(format_string("line %d: The parameter or member named by a length annotation must be an integer type", lengthAnno->getLocation().m_firstLine));
        }

        // Verify both the data and length members are the same direction.
        if (member->getDirection() != lengthMember->getDirection())
        {
            throw semantic_error(format_string("line %d: The parameter named by a length annotation must be the same direction as the data parameter", lengthAnno->getLocation().m_firstLine));
        }

        if (memberTrueType->isList())
        {
            ListType *memberListType = dynamic_cast<ListType*>(memberTrueType);
            assert(memberListType);
            memberListType->setLengthVariableName(lengthIdent);
        }

        // Record that the referenced member is the length for this member.
        lengthMember->setLengthForMember(member);
    }
}

data_map PythonGenerator::getFunctionTemplateData(Function *fn, int fnIndex)
{
    // Process @length annotations for this function.
    scanStructForLengthAnnotation(&fn->getParameters());

    data_map info;
    std::string proto = getFunctionPrototype(fn);

    info["name"] = fn->getName();
    info["prototype"] = proto;
    info["id"] = fn->getUniqueId();
    info["isOneway"] = fn->isOneway();
    info["isReturnValue"] = !fn->isOneway();
    info["mlComment"] = convertComment(fn->getMlComment(), kMultilineComment);
    info["ilComment"] = convertComment(fn->getIlComment(), kInlineComment);

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
        std::string name = param->getName();
        paramInfo["name"] = name;
        paramInfo["type"] = getTypeInfo(paramType);

        Annotation *anno = param->findAnnotation(NULLABLE_ANNOTATION);
        paramInfo["isNullable"] = (anno != nullptr);

        StructMember *otherMember = param->getLengthForMember();
        paramInfo["lengthForMember"] = (otherMember == nullptr ? "" : otherMember->getName());

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

    Log::info("    %d: (%d) %s\n", fnIndex, fn->getUniqueId(), info["prototype"]->getvalue().c_str());

    return info;
}

std::string PythonGenerator::getFunctionPrototype(Function *fn)
{
    std::string proto = fn->getName();
    proto += "(self";

    auto params = fn->getParameters().getMembers();
    if (params.size())
    {
        for (auto it : params)
        {
            // Skip params that are length for other params.
            if (it->getLengthForMember() != nullptr)
            {
                continue;
            }

            proto += ", ";
            proto += it->getName();
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
        Annotation *externAnnotation = constVar->findAnnotation(EXTERNAL_ANNOTATION);
        if (!externAnnotation)
        {
            constInfo["name"] = constVar->getName();

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
            constInfo["mlComment"] = convertComment(constVar->getMlComment(), kMultilineComment);
            constInfo["ilComment"] = convertComment(constVar->getIlComment(), kInlineComment);
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
    for (auto it : m_globals->getSymbolsOfType(DataType::kEnumTypeSymbol))
    {
        EnumType *enumType = dynamic_cast<EnumType *>(it);
        assert(enumType);
        Annotation *externAnnotation = enumType->findAnnotation(EXTERNAL_ANNOTATION);
        if (!externAnnotation)
        {
            Log::info("%d: %s\n", n, enumType->getName().c_str());
            data_map enumInfo;
            enumInfo["name"] = filterName(enumType->getName());
            enumInfo["members"] = getEnumMembersTemplateData(enumType);
            enumInfo["mlComment"] = convertComment(enumType->getMlComment(), kMultilineComment);
            enumInfo["ilComment"] = convertComment(enumType->getIlComment(), kInlineComment);
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
        enumMember["name"] = member->getName();
        enumMember["value"] = member->getValue();
        Log::info("    %d: %s = %d\n", j, member->getName().c_str(), member->getValue());
        enumMember["mlComment"] = convertComment(member->getMlComment(), kMultilineComment);
        enumMember["ilComment"] = convertComment(member->getIlComment(), kInlineComment);
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
    for (auto it : m_globals->getSymbolsOfType(DataType::kAliasTypeSymbol))
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

            // Only generate aliases for enums and structs in Python.
            if (!(trueDataType->isEnum() || trueDataType->isStruct()))
            {
                continue;
            }

            std::string realType = aliasType->getName();
            Log::info("%s\n", realType.c_str());

            // Ignore aliases added by SymbolScanner to generate struct typedefs for C.
            // TODO remove these C-specific aliases.
            if (elementDataType->getName() == aliasType->getName())
            {
                continue;
            }

            aliasInfo["name"] = filterName(realType);
            aliasInfo["elementType"] = getTypeInfo(elementDataType);
            aliasInfo["trueType"] = getTypeInfo(trueDataType);

            aliasInfo["mlComment"] = aliasType->getMlComment();
            aliasInfo["ilComment"] = aliasType->getIlComment();

            aliases.push_back(aliasInfo);
            ++n;
//        }
    }
    m_templateData["aliases"] = aliases;
}

void PythonGenerator::makeStructsTemplateData()
{
    Log::info("Structs:\n");
    data_list structs;
    for (auto it : m_globals->getSymbolsOfType(Symbol::kStructTypeSymbol))
    {
        StructType *structType = dynamic_cast<StructType *>(it);
        assert(structType);
        std::string structDesc = structType->getDescription();
        Log::info("%s\n", structDesc.c_str());

        // Process @length annotations for this struct.
        scanStructForLengthAnnotation(structType);

        data_map structInfo;
        structInfo["name"] = filterName(structType->getName());
        structInfo["mlComment"] = convertComment(structType->getMlComment(), kMultilineComment);
        structInfo["ilComment"] = convertComment(structType->getIlComment(), kInlineComment);
        setStructMembersTemplateData(structType, structInfo);

        structs.push_back(structInfo);
    }
    m_templateData["structs"] = structs;
}

void PythonGenerator::setStructMembersTemplateData(StructType *structType, cpptempl::data_map &structInfo)
{
    data_list members;
    for (auto member : structType->getMembers())
    {
        data_map member_info;
        setOneStructMemberTemplateData(member, member_info);
        members.push_back(member_info);
    }
    structInfo["members"] = members;
}

void PythonGenerator::setOneStructMemberTemplateData(StructMember *member, cpptempl::data_map &member_info)
{
    std::string memberName = member->getName();

    Annotation *anno = member->findAnnotation(NULLABLE_ANNOTATION);

    // Info for declaring struct in common header
    member_info["name"] = memberName;
    member_info["isNullable"] = (anno != nullptr);
    member_info["type"] = getTypeInfo(member->getDataType());
    member_info["mlComment"] = convertComment(member->getMlComment(), kMultilineComment);
    member_info["ilComment"] = convertComment(member->getIlComment(), kInlineComment);

    StructMember *otherMember = member->getLengthForMember();
    member_info["lengthForMember"] = (otherMember == nullptr ? "" : otherMember->getName());
}

cpptempl::data_map PythonGenerator::getTypeInfo(DataType *t)
{
    data_map info;
    info["name"] = filterName(t->getName());
    switch (t->getDataType())
    {
        case DataType::kVoidType:
            info["type"] = "void";
            break;
        case DataType::kBuiltinType:
            assert(dynamic_cast<const BuiltinType *>(t));
            info["type"] = getBuiltinTypename(dynamic_cast<const BuiltinType *>(t));
            break;
        case DataType::kListType:
        {
            const ListType *a = dynamic_cast<const ListType *>(t);
            assert(a);
            info["type"] = "list";
            info["elementType"] = getTypeInfo(a->getElementType());
            break;
        }
        case DataType::kArrayType:
        {
            // Array type requires the array element count to come after the variable/member name.
            ArrayType *a = dynamic_cast<ArrayType *>(t);
            assert(a);
            info["type"] = "array";
            info["elementCount"] = a->getElementCount();
            info["elementType"] = getTypeInfo(a->getElementType());
            break;
        }
        case DataType::kEnumType:
            info["type"] = "enum";
            break;
        case DataType::kStructType:
            info["type"] = "struct";
            break;
        case DataType::kUnionType:
        {
            UnionType *unionType = dynamic_cast<UnionType *>(t);
            assert(unionType);
            info["type"] = "union";

            // Set discriminator field name.
            std::string discriminatorName = unionType->getDiscriminatorName();
            info["discriminatorName"] = discriminatorName;

            // Fill in discriminator type info.
            Symbol *discriminatorSym = unionType->getParentStruct()->getScope().getSymbol(discriminatorName);
            if (!discriminatorSym)
            {
                throw semantic_error(format_string("unable to find union discriminator '%s' in struct", discriminatorName.c_str()));
            }
            StructMember *discriminatorMember = dynamic_cast<StructMember*>(discriminatorSym);
            if (!discriminatorMember)
            {
                throw internal_error(format_string("union discriminator is not a struct member"));
            }
            info["discriminatorType"] = getTypeInfo(discriminatorMember->getDataType());

            data_list unionCases;
            for (auto unionCase : unionType->getCases())
            {
                data_map caseData;
                caseData["name"] = unionCase->getCaseName();
                caseData["value"] = unionCase->getCaseValue();
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
        case DataType::kAliasType:
            info = getTypeInfo(t->getTrueDataType());
            break;
        default:
            throw internal_error("unknown data type");
    }
    return info;
}

std::string PythonGenerator::getBuiltinTypename(const BuiltinType *t)
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

std::string PythonGenerator::filterName(const std::string & name)
{
    std::string result = name;
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

std::string PythonGenerator::convertComment(const std::string & comment, comment_type commentType)
{
    // Longer patterns are ordered earlier than similar shorter patterns.
    static const char * const kCommentBegins[] = {
            "//!<",
            "//!",
            "///<",
            "///",
            "/*!<",
            "/*!",
            "/**<",
            "/**",
            0
        };
    static const char * const kCommentEnds[] = {
            "*/",
            0
        };

    std::string result = stripWhitespace(comment);
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
        if (p != std::string::npos)
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
        if (p != std::string::npos)
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
        // Find the next newline.
        p = result.find('\n', p + 1);
        if (p == std::string::npos)
        {
            break;
        }
        // Skip over the newline that we want to keep.
        ++p;

        // Erase over any whitespace, except newlines.
        n = 0;
        for (; p + n < result.size(); ++n)
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
        if (result[p] == '/' && result[p+1] == '/' && result[p+2] == '!')
        {
            result.erase(p, 3);
        }
        else if (result[p] == '/' && result[p+1] == '/')
        {
            result.erase(p, 2);
        }
        else if (result[p] == '*')
        {
            result.erase(p, 1);
        }

        // Insert Python line comment start.
        result.insert(p, "#");
    }

    // Return comment converted to Python form.
    return (commentType == kInlineComment
        ? "# " + result
        : "# " + result); // "\"\"\"" + result + " \"\"\"");
}

std::string PythonGenerator::stripWhitespace(const std::string & s)
{
    std::string result = s;
    int32_t i;
    uint32_t n;

    // Strip leading whitespace.
    n = 0;
    for (i = 0; i < result.size(); ++i, ++n)
    {
        char c = result[i];
        if (c != ' ' && c != '\t' && c != '\n' && c != '\r')
        {
            break;
        }
    }
    if (n > 0)
    {
        result.erase(0, n);
    }

    // Strip trailing whitespace.
    n = 0;
    for (i = result.size() - 1; i > 0; --i, ++n)
    {
        char c = result[i];
        if (c != ' ' && c != '\t' && c != '\n' && c != '\r')
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

