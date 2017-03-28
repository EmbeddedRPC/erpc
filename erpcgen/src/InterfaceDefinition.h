/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
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

#ifndef _EMBEDDED_RPC__INTERFACEDEFINITION_H_
#define _EMBEDDED_RPC__INTERFACEDEFINITION_H_

#include "AstNode.h"
#include "types/Program.h"
#include "types/SymbolScope.h"
#include <boost/filesystem.hpp>
#include <iostream>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen {

/*!
 * @brief Collects all definitions from an IDL.
 *
 * Also directs the IDL parsing and code generation process.
 */
class InterfaceDefinition
{
public:
    enum _error_handling_checks
    {
        kAll,
        kInfraErrors,
        kAllocErrors,
        kNone
    };

    /*!
     * @brief Default constructor.
     */
    InterfaceDefinition();

    /*!
     * @brief Destructor.
     */
    virtual ~InterfaceDefinition() {}

    /*!
     * @brief This function initialize symbol scope variable m_globals.
     *
     * @see void InterfaceDefinition::createBuiltinTypes()
     */
    void init();

    /*!
     * @brief This function parse given IDL file.
     *
     * File content is split into AstNode three. Then dump AstNode three.
     * Then is called symbol scanner to walk AstNode three.
     *
     * @param[in] inputFile File, which is parsed.
     *
     * @exception std::runtime_error Thrown if is bad return result for parse function or no ast node three is created.
     */
    void parse(const char *inputFile);

    /*!
     * @brief This function returns node belong to this object.
     *
     * return AstNode * Node belong to this object.
     */
    AstNode *getAst() { return m_ast; }

    /*!
     * @brief This function returns global symbol scope.
     *
     * return SymbolScope & Global symbol scope.
     */
    SymbolScope &getGlobals() { return m_globals; }

    /*!
     * @brief This function sets program information.
     *
     * This function sets out file name, output path and flag for generating each interface
     * into its set of output files if requested.
     *
     * @param[in] fileName Necessary for output files names.
     * @param[in] outputFilePath Path to directory, where files will be generated.
     */
    void setProgramInfo(const std::string &fileName, const std::string &outputFilePath);

    /*!
     * @brief This function sets program name.
     *
     * @param[in] programName Name of program.
     */
    void setProgramName(const std::string &programName) { m_programName = programName; }

    /*!
     * @brief This function returns program name.
     *
     * @return Return name of program.
     */
    const std::string getProgramName() { return programSymbol()->getName(); }

    /*!
     * @brief This function returns output directory path.
     *
     * @return Return output directory path.
     */
    boost::filesystem::path getOutputDirectory() { return m_outputDirectory; }

    /*!
     * @brief This function returns output file name.
     *
     * @return Return name of output file.
     */
    std::string getOutputFilename() { return m_outputFilename; }

    /*!
     * @brief This function sets output file name.
     *
     * @param[in] filename Output file name.
     */
    void setOutputFilename(const std::string &filename);

    /*!
     * @brief This function returns information if program symbol was set in parsed file.
     *
     * @retval true When program was set in parsed file.
     * @retval true When program wasn't set in parsed file.
     */
    bool hasProgramSymbol();

    /*!
     * @brief This function returns program symbol.
     *
     * @return Pointer to program symbol.
     */
    Program *programSymbol();

    /*!
     * @brief This functions sets type of checks which should be present in generated file.
     */
    void setErrorHandlingChecksType();

    /*!
     * @brief This functions returns type of checks which should be present in generated file.
     */
    _error_handling_checks getErrorHandlingChecksType() { return m_error_handling_check; }

private:
    /* Instance Variables */
    AstNode *m_ast;                                /*!< Root of AstNode tree. */
    SymbolScope m_globals;                         /*!< Symbol scope data. */
    std::string m_programName;                     /*!< Program name set via parsed file. */
    std::string m_outputFilename;                  /*!< Output file name. */
    boost::filesystem::path m_outputDirectory;     /*!< Output file path. */
    _error_handling_checks m_error_handling_check; /*!< Type of generated error checks */

    /* Private Functions */
    /*!
     * @brief This function add builtin types into symbol scope variable m_globals.
     */
    void createBuiltinTypes();

    /*!
     * @brief Sets the output directory for generated erpc files.
     *
     * This function is used to initialize the outputDirectory instance
     * variable for the program. It uses the output directory
     * given on the command line, if any, and then appends the output
     * directory given by a program annotation, if any.
     *
     * @param[in] outputDir Output directory from command line args.
     */
    void setOutputDirectory(const std::string &outputDir);
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__INTERFACEDEFINITION_H_
