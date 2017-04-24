/*
 * Copyright (c) 2014-2015, Freescale Semiconductor, Inc.
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
    /*!
     * @brief Constructor.
     *
     * Interface definition contains all information about parsed files and builtin types.
     *
     * @param[in] def Contains all Symbols parsed from IDL files.
     * @param[in] idlCrc16 Crc16 of IDL files.
     */
    Generator(InterfaceDefinition *def, uint16_t idlCrc16);

    /*!
     * @brief Destructor.
     */
    virtual ~Generator() {}

    /*!
     * @brief Generate output files.
     */
    virtual void generate() = 0;

protected:
    uint16_t m_idlCrc16;               /*!< Storing crc16 of IDL files and erpcgen verssion. */
    cpptempl::data_map m_templateData; /*!< Data prepared for templates files. */

    InterfaceDefinition *m_def; /*!< Interface definitions. */
    SymbolScope *m_globals;     /*!< Symbol scope data. */
    typedef std::map<std::string, std::vector<cpptempl::data_ptr> >
        interfaceLists_t; /*!< Contains list of interfaces belonging to different groups. */

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
    void generateOutputFile(const std::string &fileName,
                            const std::string &templateName,
                            cpptempl::data_map &templateData,
                            const char *const kParseFile);

    /*!
     * @brief This function generate output crc16 source file.
     */
    virtual void generateCrcFile() = 0;

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
                                             const std::string &referenceName,
                                             const std::string &annName);

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
    StructMember *findParamReferencedFrom(const StructType::member_vector_t &members,
                                          const std::string &referenceName);

    /*!
     * @brief This function return actual time string representation.
     *
     * @return Actual time.
     */
    std::string getTime();

    /*!
     * @brief This function sets interfaces template data.
     *
     * This function sets interfaces template data with all data, which
     * are necessary for generating output code for output files.
     */
    interfaceLists_t makeInterfacesTemplateData();

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
    cpptempl::data_list getFunctionsTemplateData(Interface *iface);

    /*!
     * @brief This function return interface function template data.
     *
     * This function return interface function template data with all data, which
     * are necessary for generating output code for output files.
     *
     * @param[in] fn From this are set interface function template data.
     * @param[in] fnIndex Function index.
     *
     * @return Contains interface function data.
     */
    virtual cpptempl::data_map getFunctionTemplateData(Function *fn, int fnIndex) = 0;

    /*!
     * @brief This function will get symbol comments and convert to language specific ones
     *
     * @param[in] symbol Pointer to symbol.
     * @param[inout] symbolInfo Data map, which contains information about symbol.
     */
    virtual void setTemplateComments(Symbol *symbol, cpptempl::data_map &symbolInfo) = 0;

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
     * @param[inout] templateData Data prepared for templates files
     * @param[in] interfaceLists Lists of interfaces sorted by map name.
     */
    void generateInterfaceOutputFiles(cpptempl::data_map &templateData, interfaceLists_t interfaceLists);

    /*!
     * @brief This function will insert given interface map into vector of interfaces based on given map name.
     *
     * @param[inout] interfaceLists Lists of interfaces sorted by map name.
     * @param[in] interfaceMap Interface data map information.
     * @param[in] mapName Name used for sorting interface data maps.
     */
    void fillInterfaceListsWithMap(interfaceLists_t &interfaceLists,
                                   cpptempl::data_ptr interfaceMap,
                                   std::string mapName);

    /*!
     * @brief This function sets template data for include directives from an IDL file
     *
     * @param[inout] templateData Data prepared for templates files
     */
    void makeIncludesTemplateData(cpptempl::data_map &templateData);
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__GENERATOR_H_
