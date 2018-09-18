/*
 * Copyright (c) 2014-2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
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
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

Generator::Generator(InterfaceDefinition *def, generator_type_t generatorType)
: m_def(def)
, m_globals(&(def->getGlobals()))
, m_idlCrc16(def->getIdlCrc16())
, m_generatorType(generatorType)
{
    m_templateData["erpcVersion"] = ERPC_VERSION;
    m_templateData["erpcVersionNumber"] = ERPC_VERSION_NUMBER;

    // crc of erpcgen version and idl files.
    m_templateData["crc16"] = "";

    m_templateData["todaysDate"] = getTime();

    m_templateData["sharedMemBeginAddr"] = "";
    m_templateData["sharedMemEndAddr"] = "";
    m_templateData["dynamicServices"] = false;

    m_outputDirectory = m_def->getOutputDirectory();

    if (m_def->hasProgramSymbol())
    {
        Log::info("program: ");
        Log::info("%s\n", m_def->getOutputFilename().c_str());

        Program *program = m_def->getProgramSymbol();

        /* Shared memory area. */
        Value *sharedMemBValue = getAnnValue(program, SHARED_MEMORY_BEGIN_ANNOTATION);
        Value *sharedMemEValue = getAnnValue(program, SHARED_MEMORY_END_ANNOTATION);
        if (sharedMemBValue && sharedMemEValue)
        {
            m_templateData["sharedMemBeginAddr"] = sharedMemBValue->toString();
            m_templateData["sharedMemEndAddr"] = sharedMemEValue->toString();
            Log::warning("Shared memory is supported only for C language used on embedded devices.\n");
        }
        else if (sharedMemBValue || sharedMemEValue)
        {
            throw semantic_error(
                "Annotations @shared_memory_begin and @shared_memory_end both (or no one) need exists and contains "
                "addresses.");
        }

        if (findAnnotation(program, CRC_ANNOTATION) != nullptr)
        {
            m_templateData["crc16"] = m_idlCrc16;
        }

        if (findAnnotation(program, DYNAMIC_SERVICES_ANNOTATION) != nullptr)
        {
            m_templateData["dynamicServices"] = true;
        }

        m_outputDirectory /= getAnnStringValue(m_def->getProgramSymbol(), OUTPUT_DIR_ANNOTATION);
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
        vector<Annotation *> groupAnns = getAnnotations(iface, GROUP_ANNOTATION);
        if (!groupAnns.empty())
        {
            for (auto groupAnnIt : groupAnns)
            {
                string name = (groupAnnIt->hasValue()) ? groupAnnIt->getValueObject()->toString() : "";
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

    // set codec information
    switch (m_def->getCodecType())
    {
        case InterfaceDefinition::kBasicCodec:
        {
            m_templateData["codecClass"] = "BasicCodec";
            m_templateData["codecHeader"] = "erpc_basic_codec.h";
            break;
        }
        default:
        {
            m_templateData["codecClass"] = "Codec";
            m_templateData["codecHeader"] = "erpc_codec.h";
            break;
        }
    }
}

Group *Generator::getGroupByName(string name)
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

void Generator::openFile(ofstream &fileOutputStream, const string &fileName)
{
    if (!m_outputDirectory.empty())
    {
        // TODO: do we have to create a copy of the outputDir here? Doesn't make sense...
        boost::filesystem::path dir(m_outputDirectory);
        if (!boost::filesystem::is_directory(dir))
        {
            // Create_directories function return false also when it create new directory.
            // It is in case, when directory ends with slash. For these case is better use is_directory for check if
            // directories are created.
            boost::filesystem::create_directories(dir);
            if (!boost::filesystem::is_directory(dir))
            {
                throw runtime_error(format_string("could not create directory path '%s'", m_outputDirectory.c_str()));
            }
        }
    }
    string filePathWithName = (m_outputDirectory / fileName).string();
    // Open file.
    fileOutputStream.open(filePathWithName, ios::out | ios::binary);
    if (!fileOutputStream.is_open())
    {
        throw runtime_error(format_string("could not open output file '%s'", filePathWithName.c_str()));
    }
}

void Generator::generateOutputFile(const string &fileName, const string &templateName, data_map &templateData,
                                   const char *const kParseFile)
{
    ofstream fileOutputStream;
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

string Generator::stripExtension(const string &filename)
{
    auto result = filename.rfind('.');
    if (result != string::npos)
    {
        return filename.substr(0, result);
    }
    else
    {
        return filename;
    }
}

StructMember *Generator::findParamReferencedFromAnn(const StructType::member_vector_t &members,
                                                    const string &referenceName, const string &annName)
{
    for (StructMember *structMember : members)
    {
        string lengthName = getAnnStringValue(structMember, annName);
        if (strcmp(lengthName.c_str(), referenceName.c_str()) == 0)
        {
            return structMember;
        }
    }
    return nullptr;
}

StructMember *Generator::findParamReferencedFromUnion(const StructType::member_vector_t &members,
                                                      const string &referenceName)
{
    for (StructMember *structMember : members)
    {
        DataType *trueDataType = structMember->getDataType()->getTrueDataType();
        if (trueDataType->isUnion())
        {
            UnionType *unionType = dynamic_cast<UnionType *>(trueDataType);
            assert(unionType);
            if (unionType->isNonEncapsulatedUnion())
            {
                string lengthName = getAnnStringValue(structMember, DISCRIMINATOR_ANNOTATION);
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
                                                 const string &referenceName)
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

string Generator::getTime()
{
    time_t now = time(nullptr);
    string nowString = ctime(&now);
    nowString.pop_back(); // Remove trailing newline.
    return nowString;
}

DataType *Generator::findChildDataType(set<DataType *> &dataTypes, DataType *dataType)
{
    // Detecting loops from forward declarations.
    // Insert data type into set
    if (!dataTypes.insert(dataType).second)
    {
        return dataType;
    }

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
                for (auto unionMember : unionType->getUnionMembers().getMembers())
                {
                    findChildDataType(dataTypes, unionMember->getDataType());
                }
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

void Generator::findGroupDataTypes()
{
    for (Group *group : m_groups)
    {
        for (Interface *iface : group->getInterfaces())
        {
            for (Function *fn : iface->getFunctions())
            {
                // handle return value
                set<DataType *> dataTypes;
                StructMember *structMember = fn->getReturnStructMemberType();
                DataType *transformedDataType = findChildDataType(dataTypes, fn->getReturnType());
                structMember->setDataType(transformedDataType);

                // save all transformed data types directions into data type map
                if (!findAnnotation(fn->getReturnType(), SHARED_ANNOTATION))
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

                    mit->setDataType(findChildDataType(dataTypes, mit->getDataType()));

                    // save all transformed data types directions into data type map
                    if (!findAnnotation(mit, SHARED_ANNOTATION))
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
        ifaceInfo["name"] = make_data(getOutputName(iface));
        ifaceInfo["id"] = data_ptr(iface->getUniqueId());

        setTemplateComments(iface, ifaceInfo);

        // TODO: for C only?
        ifaceInfo["serviceClassName"] = getOutputName(iface) + "_service";

        Log::info("%d: (%d) %s\n", n++, iface->getUniqueId(), iface->getName().c_str());

        /* Has interface function declared as non-external? */
        data_list functions = getFunctionsTemplateData(group, iface);
        ifaceInfo["functions"] = functions;
        ifaceInfo["isNonExternalInterface"] = false;
        for (int i = 0; i < functions.size(); ++i)
        {
            assert(dynamic_cast<DataMap *>(functions[i].get().get()));
            string isNonExternalFunction =
                dynamic_cast<DataMap *>(functions[i].get().get())->getmap()["isNonExternalFunction"]->getvalue();
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

void Generator::generateGroupOutputFiles(Group *group)
{
    // generate output files only for groups with interfaces or for IDLs with no interfaces at all
    if (!group->getInterfaces().empty() || (m_groups.size() == 1 && group->getName() == ""))
    {
        string groupName = group->getName();
        string fileName = stripExtension(m_def->getOutputFilename());
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
        for (auto include : getAnnotations(m_def->getProgramSymbol(), INCLUDE_ANNOTATION))
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
    set<string> tempSet;

    for (Interface *iface : group->getInterfaces())
    {
        Annotation *includeAnn = findAnnotation(iface, INCLUDE_ANNOTATION);
        if (includeAnn)
        {
            string include = includeAnn->getValueObject()->toString();
            if (tempSet.find(include) == tempSet.end())
            {
                includes.push_back(include);
                tempSet.insert(include);
            }
        }

        for (Function *func : iface->getFunctions())
        {
            Annotation *funcAnn = findAnnotation(func, INCLUDE_ANNOTATION);
            if (funcAnn)
            {
                string include = funcAnn->getValueObject()->toString();
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

bool Generator::isMemberDataTypeUsingForwardDeclaration(Symbol *topSymbol, Symbol *memberSymbol)
{
    return (m_globals->getSymbolPos(topSymbol) < m_globals->getSymbolPos(memberSymbol));
}

string Generator::getOutputName(Symbol *symbol, bool check)
{
    string annName;
    uint32_t line;
    Annotation *ann = findAnnotation(symbol, NAME_ANNOTATION);
    if (ann)
    {
        annName = ann->getValueObject()->toString();
        if (annName.empty())
        {
            throw semantic_error(format_string("Missing value for annotation named @%s on line '%d'", NAME_ANNOTATION,
                                               ann->getLocation().m_firstLine));
        }
        Log::warning(
            "line %d: Be careful when @%s annotation is used. This can cause compile issue. See documentation.\n",
            ann->getLocation().m_firstLine, NAME_ANNOTATION);
        line = ann->getLocation().m_firstLine;
    }
    else
    {
        annName = symbol->getName();
        line = symbol->getFirstLine();
    }

    if (check)
    {
        auto it = reserverdWords.find(annName);
        if (it != reserverdWords.end())
        {
            throw semantic_error(
                format_string("line %d: Wrong symbol name '%s'. Cannot use program language reserved words.", line,
                              annName.c_str())
                    .c_str());
        }
    }

    return annName;
}

Annotation::program_lang_t Generator::getAnnotationLang()
{
    if (m_generatorType == kC)
    {
        return Annotation::kC;
    }
    else if (m_generatorType == kPython)
    {
        return Annotation::kPython;
    }

    throw internal_error("Unsupported generator type specified for annotation.");
}

Annotation *Generator::findAnnotation(Symbol *symbol, string name)
{
    return symbol->findAnnotation(name, getAnnotationLang());
}

vector<Annotation *> Generator::getAnnotations(Symbol *symbol, string name)
{
    return symbol->getAnnotations(name, getAnnotationLang());
}

Value *Generator::getAnnValue(Symbol *symbol, string name)
{
    return symbol->getAnnValue(name, getAnnotationLang());
}

string Generator::getAnnStringValue(Symbol *symbol, string name)
{
    return symbol->getAnnStringValue(name, getAnnotationLang());
}

data_list Generator::getFunctionsTemplateData(Group *group, Interface *iface)
{
    data_list fns;

    int j = 0;
    for (auto fit : iface->getFunctions())
    {
        data_map function = getFunctionTemplateData(group, fit);
        fns.push_back(function);

        Log::info("    %d: (%d) %s\n", j, fit->getUniqueId(), function["prototype"]->getvalue().c_str());
    }
    return fns;
}
