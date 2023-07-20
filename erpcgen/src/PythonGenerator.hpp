/*
 * Copyright (c) 2014-2016 Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__PYTHONGENERATOR_H_
#define _EMBEDDED_RPC__PYTHONGENERATOR_H_

#include "Generator.hpp"
#include "cpptempl.hpp"

#include <set>
#include <string>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen {

/*!
 * @brief Code generator for Python.
 */
class PythonGenerator : public Generator
{
public:
    /*!
     * @brief This function is constructor of PythonGenerator class.
     *
     * @param[in] def Contains all Symbols parsed from IDL files.
     */
    explicit PythonGenerator(InterfaceDefinition *def);

    /*!
     * @brief This function is destructor of PythonGenerator class.
     *
     * This function close opened files.
     */
    virtual ~PythonGenerator() {}

    /*!
     * @brief This function generate output code for output files.
     *
     * This code call all necessary functions for prepare output code and parse it into output files.
     */
    virtual void generate() override;

protected:
    std::string m_suffixStrip; //!< String to remove from suffixes of names.
    size_t m_suffixStripSize;  //!< Length of the suffix filter string.

    /*!
     * @brief This function prepare helpful functions located in template files.
     *
     * These function may be used while parsing templates
     */
    void parseSubtemplates();

    /*!
     * @brief This function generate output files.
     *
     * This function call functions for generating client/server source files.
     *
     * @param[in] fileName Name for output files.
     */
    void generateOutputFiles(const std::string &fileName) override;

    /*!
     * @brief This function generate output common module.
     *
     * @param[in] fileName Name for output files.
     */
    void generateCommonFile(std::string fileName);

    /*!
     * @brief This function generate output package init file.
     *
     * @param[in] fileName Name for output files.
     */
    void generateInitFile(std::string fileName);

    /*!
     * @brief This function generate output client file.
     *
     * @param[in] fileName Name for output files.
     */
    void generateClientFile(std::string fileName);

    /*!
     * @brief This function generate output server file.
     *
     * @param[in] fileName Name for output files.
     */
    void generateServerFile(std::string fileName);

    /*!
     * @brief This function generate output interface file.
     *
     * @param[in] fileName Name for output files.
     */
    void generateInterfaceFile(std::string fileName);

    /*!
     * @brief This function generate output crc16 source file.
     */
    virtual void generateGlobalInitFile();

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
    cpptempl::data_map getFunctionTemplateData(Group *group, Function *fn, Interface *interface = nullptr) override;

    /*!
     * @brief This function will get symbol comments and convert to language specific ones
     *
     * @param[in] symbol Pointer to symbol.
     * @param[inout] symbolInfo Data map, which contains information about symbol.
     */
    void setTemplateComments(Symbol *symbol, cpptempl::data_map &symbolInfo) override;

    /*!
     * @brief This function return interface function prototype.
     *
     * @param[in] fn Function for prototyping.
     *
     * @return String prototype representation for given function.
     */
    std::string getFunctionPrototype(Function *fn);

    /*!
     * @brief This function sets const template data.
     *
     * This function sets const template data with all data, which
     * are necessary for generating output code for output files.
     */
    void makeConstTemplateData();

    /*!
     * @brief This function sets enum template data.
     *
     * This function sets enum template data with all data, which
     * are necessary for generating output code for output files.
     */
    void makeEnumsTemplateData();

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
    cpptempl::data_map makeGroupSymbolsTemplateData(Group *group) override;

    /*!
     * @brief This function return enum members template data.
     *
     * This function return enum members template data with all data, which
     * are necessary for generating output code for output files.
     *
     * @param[in] enumType Pointer to enum.
     *
     * @return Contains enum members data.
     */
    cpptempl::data_list getEnumMembersTemplateData(EnumType *enumType);

    /*!
     * @brief Fill in template data for type aliases.
     */
    void makeAliasesTemplateData();

    /*!
     * @brief This function sets function type template data.
     *
     * This is used for registering callback functions in generated output.
     */
    void makeFunctionsTemplateData();

    /*!
     * @brief This function sets struct member information to struct data map variable.
     *
     * This function sets struct member information to struct data map variable, which
     * are necessary for generating output code for output files.
     *
     * @param[in] structType Struct, which contains struct members.
     * @param[in,out] structInfo Data map, which contains information about struct and struct members.
     */
    void setStructMembersTemplateData(StructType *structType, cpptempl::data_map &structInfo);

    /*!
     * @brief Fill in template data for a single struct member.
     */
    void setOneStructMemberTemplateData(StructMember *member, cpptempl::data_map &member_info);

    /*!
     * @brief This function return necessary template data for data type.
     *
     * This function return data type template data with all necessary data, which
     * are necessary for generating output code for output files.
     *
     * @param[in] t Data type. From this are set data type template data.
     *
     * @return Contains data type template data.
     */
    cpptempl::data_map getTypeInfo(DataType *t);

    /*!
     * @brief This function return string representation for given BuiltinType.
     *
     * @param[in] t Builtin type.
     *
     * @return String representation for given builtin type.
     */
    std::string getBuiltinTypename(const BuiltinType *t);

    /*!
     * @brief Filter symbol names.
     */
    std::string filterName(const std::string &name);

    //! @brief Possible Doxygen comment styles.
    enum comment_type
    {
        kMultilineComment, //!< Leading multi-line comment
        kInlineComment,    //!< Trailing inline comment.
    };

    /*!
     * @brief Converts a C++-style Doxygen comment into a Python Doxygen comment.
     *
     * @param[inout] comment The C++ comment.
     * @param[in] commentType Inline or multiline comment.
     *
     * @return Python form of the provided comment.
     */
    std::string convertComment(const std::string &comment, comment_type commentType);

    /*!
     * @brief Strip leading and trailing whitespace.
     *
     * @param[in] s String from which are stripped white spaces.
     */
    std::string stripWhitespace(const std::string &s);

    /*!
     * @brief Check if character is whitespace type.
     *
     * @param[in] c Checked character.
     */
    bool checkWhitspaceChar(char c);

    /*!
     * Stores reserved words for Python program language.
     */
    void initPythonReservedWords();
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__PYTHONGENERATOR_H_
