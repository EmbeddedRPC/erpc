/*
 * Copyright (c) 2014-2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__GENERATOR_H_
#define _EMBEDDED_RPC__GENERATOR_H_

#include "InterfaceDefinition.h"
#include "cpptempl.h"
#include "types/AliasType.h"
#include "types/ArrayType.h"
#include "types/BuiltinType.h"
#include "types/ConstType.h"
#include "types/DataType.h"
#include "types/EnumMember.h"
#include "types/EnumType.h"
#include "types/Function.h"
#include "types/FunctionType.h"
#include "types/Group.h"
#include "types/Interface.h"
#include "types/ListType.h"
#include "types/StructMember.h"
#include "types/StructType.h"
#include "types/UnionType.h"
#include "types/VoidType.h"
#include <fstream>
#include <string>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen {

/*!
 * @brief Abstract code generator base class.
 */
class Generator
{
public:
    enum generator_type_t
    {
        kC,
        kPython
    }; /*!< Type of generator. */

    /*!
     * @brief Constructor.
     *
     * Interface definition contains all information about parsed files and builtin types.
     *
     * @param[in] def Contains all Symbols parsed from IDL files.
     */
    Generator(InterfaceDefinition *def, generator_type_t generatorType);

    /*!
     * @brief Destructor.
     */
    virtual ~Generator() {}

    /*!
     * @brief Generate output files.
     */
    virtual void generate() = 0;

protected:
    uint16_t m_idlCrc16;                       /*!< Storing crc16 of IDL files and erpcgen version. */
    cpptempl::data_map m_templateData;         /*!< Data prepared for templates files. */
    InterfaceDefinition *m_def;                /*!< Interface definitions. */
    SymbolScope *m_globals;                    /*!< Symbol scope data. */
    std::vector<Group *> m_groups;             /*!< List of groups. */
    std::set<std::string> reserverdWords;      /*!< Program language reserved words. */
    generator_type_t m_generatorType;          /*!< Type of generator. */
    boost::filesystem::path m_outputDirectory; /*!< Output file path. */
    /*!
     * @brief This function open file
     *
     * @param[in] fileName Output file name.
     * @param[inout] fileOutputStream Opened output file stream.
     *
     * @exception std::runtime_error Thrown, when can't create directory.
     * @exception std::runtime_error Thrown, when file is not open.
     */
    void openFile(std::ofstream &fileOutputStream, const std::string &fileName);

    /*!
     * @brief This function open file, generate output into this file and close this file.
     *
     * @param[in] fileName Output file name.
     * @param[in] templateName Name of template file, which is used for parser.
     * @param[in] templateData Template data used for parser.
     * @param[in] kParseFile Templates strings converted from text file by txt_to_c.py used for parser.
     *
     * @exception TemplateException Thrown, when parse template data into output is not successfully.
     */
    void generateOutputFile(const std::string &fileName, const std::string &templateName,
                            cpptempl::data_map &templateData, const char *const kParseFile);

    /*!
     * @brief
     *
     * @param[in] filename
     *
     * @return Filename with stripped extension
     */
    std::string stripExtension(const std::string &filename);

    /*!
     * @brief This function will return pointer to function parameter/structure member where given reference name is
     * referenced for.
     *
     * @param[in] members Members contain references members and searched members.
     * @param[in] referenceName Name of reference for which is member searched.
     * @param[in] annName Annotation name for which is member searched.
     *
     * @return Searched member.
     */
    StructMember *findParamReferencedFromAnn(const StructType::member_vector_t &members,
                                             const std::string &referenceName, const std::string &annName);

    /*!
     * @brief This function will return pointer to function parameter/structure member where given reference name is
     * referenced for.
     *
     * @param[in] members Members contain references members and searched members.
     * @param[in] referenceName Name of reference for which is member searched.
     *
     * @return Searched member.
     */
    StructMember *findParamReferencedFromUnion(const StructType::member_vector_t &members,
                                               const std::string &referenceName);

    /*!
     * @brief This function will return pointer to function parameter/structure member where given reference name is
     * referenced for. Combines findParamReferencedFromUnion and findParamReferencedFromAnn.
     *
     * @param[in] members Members contain references members and searched members.
     * @param[in] referenceName Name of reference for which is member searched.
     *
     * @return Searched member.
     */
    StructMember *findParamReferencedFrom(const StructType::member_vector_t &members, const std::string &referenceName);

    /*!
     * @brief This function return actual time string representation.
     *
     * @return Actual time.
     */
    std::string getTime();

    /*!
     * @brief This function finds all group's data types.
     */
    void findGroupDataTypes();

    /*!
     * @brief This function finds child data types of specified data type.
     *
     * This function should be overridden by language specific generator, if there is a need to transform data types.
     *
     * @param[inout] dataTypes Vector of found data types.
     * @param[in] dataType Top data type.
     *
     * @return Pointer to given or new DataType.
     */
    virtual DataType *findChildDataType(std::set<DataType *> &dataTypes, DataType *dataType);

    /*!
     * @brief This function transform binary data type to list and set annotation to it.
     *
     * @param[in] structMember Structure member, Function parameter or Union member.
     */
    virtual void setBinaryList(StructMember *structMember){};

    /*!
     * @brief This function sets group symbols template data.
     *
     * This function sets group symbols template data with all data, which
     * are necessary for generating output code for output files.
     *
     * @param[in] group Pointer to a group.
     *
     * @return Data map with group symbols templates.
     */
    virtual cpptempl::data_map makeGroupSymbolsTemplateData(Group *group) = 0;

    /*!
     * @brief This function return interface function template data.
     *
     * This function return interface function template data with all data, which
     * are necessary for generating output code for output files.
     *
     * @param[in] group Pointer to a group.
     * @param[in] fn From this are set interface function template data.
     * @param[in] fnIndex Function index.
     *
     * @return Contains interface function data.
     */
    virtual cpptempl::data_map getFunctionTemplateData(Group *group, Function *fn) = 0;

    /*!
     * @brief This function will get symbol comments and convert to language specific ones
     *
     * @param[in] symbol Pointer to symbol.
     * @param[inout] symbolInfo Data map, which contains information about symbol.
     */
    virtual void setTemplateComments(Symbol *symbol, cpptempl::data_map &symbolInfo) = 0;

    /*!
     * @brief This function sets interfaces template data.
     *
     * This function sets interfaces template data with all data, which
     * are necessary for generating output code for output files.
     *
     * @param[in] group Pointer to a group.
     *
     * @return Data list of interfaces templates.
     */
    cpptempl::data_list makeGroupInterfacesTemplateData(Group *group);

    /*!
     * @brief This function generates output files.
     *
     * This function call functions for generating client/server header/source files.
     *
     * @param[in] fileNameExtension Extension for file name (for example for case that each interface will be generated
     * in its
     * set of output files).
     */
    virtual void generateOutputFiles(const std::string &fileNameExtension) = 0;

    /*!
     * @brief This function generates output files for defined interfaces.
     *
     * @param[in] group Pointer to a group.
     */
    void generateGroupOutputFiles(Group *group);

    /*!
     * @brief This function sets template data for include directives from an IDL file
     */
    void makeIncludesTemplateData();

    /*!
     * @brief This function sets template data for group include directives
     *
     * @param[in] group Pointer to a group.
     *
     * @return List with all group related includes.
     */
    cpptempl::data_list makeGroupIncludesTemplateData(Group *group);

    /*!
     * @brief This function returns pointer to a group with specified name.
     *
     * @param[in] name Name of the searched group.
     *
     * @return Pointer to a group with specified name.
     */
    Group *getGroupByName(std::string name);

    /*!
     * @brief This function returns information if member data type symbol is using forward declared type.
     *
     * @param[in] topSymbol Symbol data type which is using as a member memberSymbol data type.
     * @param[in] memberSymbol Member symbol data type of topSymbol.
     *
     * @retval True when memberSymbol is using forward declared type.
     * @retval False when memberSymbol is not using forward declared type.
     */
    bool isMemberDataTypeUsingForwardDeclaration(Symbol *topSymbol, Symbol *memberSymbol);

    /*!
     * @brief This function returns symbol output name.
     * Can be different to getName() when @name is used.
     *
     * @param[in] symbol Symbol.
     * @param[in] check Check if output name is not reserved names.
     *
     * @returns Return symbol output name.
     */
    std::string getOutputName(Symbol *symbol, bool check = true);

    /*!
     * @brief Returns Generator flag used for annotation.
     *
     * @return Generator flag used for annotation.
     */
    Annotation::program_lang_t getAnnotationLang();

    /*!
     * @brief Find annotation in the annotation list
     *
     * @param[in] symbol Symbol from which is annotation extracted.
     * @param[in] name Annotation name.
     *
     * @return An index into the annotation list
     */
    Annotation *findAnnotation(Symbol *symbol, std::string name);

    /*!
     * @brief Find annotations matching name in the annotation list
     *
     * @param[in] symbol Symbol from which is annotation extracted.
     * @param[in] name Annotation name.
     *
     * @return A vector of matching annotations
     */
    std::vector<Annotation *> getAnnotations(Symbol *symbol, std::string name);

    /*!
     * @brief This function search and returns Value object for given annotation name.
     *
     * @param[in] symbol Symbol from which is annotation extracted.
     * @param[in] name Given annotation name.
     *
     * @return NULL if annotation is not found else value object.
     */
    Value *getAnnValue(Symbol *symbol, std::string name);

    /*!
     * @brief This function search and returns string for given annotation name.
     *
     * @param[in] symbol Symbol from which is annotation extracted.
     * @param[in] name Given annotation name.
     *
     * @return empty string if annotation is not found else string value.
     */
    std::string getAnnStringValue(Symbol *symbol, std::string name);

private:
    /*!
     * @brief This function return interface functions list.
     *
     * This function return interface functions list with all data, which
     * are necessary for generating output code for output files.
     *
     * @param[in] iface Pointer to interface.
     *
     * @return Contains interface functions data.
     */
    cpptempl::data_list getFunctionsTemplateData(Group *group, Interface *iface);
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__GENERATOR_H_
