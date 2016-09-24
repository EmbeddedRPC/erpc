/*
 * Copyright (c) 2014-2015, Freescale Semiconductor, Inc.
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

#ifndef _EMBEDDED_RPC__GENERATOR_H_
#define _EMBEDDED_RPC__GENERATOR_H_

#include "cpptempl.h"
#include "InterfaceDefinition.h"
#include <string>
#include <fstream>
#include "types/Function.h"
#include "types/Interface.h"
#include "types/ListType.h"
#include "types/ArrayType.h"
#include "types/EnumType.h"
#include "types/EnumMember.h"
#include "types/StructType.h"
#include "types/StructMember.h"
#include "types/VoidType.h"
#include "types/DataType.h"
#include "types/BuiltinType.h"
#include "types/AliasType.h"
#include "types/ConstType.h"
#include "types/UnionType.h"

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen
{
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
     * @param[in] def Pointer to interface definition.
     */
    Generator(InterfaceDefinition *def);

    /*!
     * @brief Destructor.
     */
    virtual ~Generator() {}
    /*!
     * @brief Generate output files.
     */
    virtual void generate() = 0;

protected:
    InterfaceDefinition *m_def; /*!< Interface definitions. */
    SymbolScope *m_globals;     /*!< Symbol scope data. */

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
    StructMember *findParamReferencedFrom(const StructType::member_vector_t &members,
                                          const std::string &referenceName,
                                          const std::string &annName);

    /*!
     * @brief This function return actual time string representation.
     *
     * @return Actual time.
     */
    std::string getTime();
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__GENERATOR_H_
