/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__INTERFACEDEFINITION_H_
#define _EMBEDDED_RPC__INTERFACEDEFINITION_H_

#include "AstNode.hpp"
#include "types/Program.hpp"
#include "types/SymbolScope.hpp"

#include <filesystem>
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
    enum class codec_t
    {
        kNotSpecified,
        kBasicCodec,
    }; /*!< Used codec type. */

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
     * @return AstNode * Node belong to this object.
     */
    AstNode *getAst() { return m_ast; }

    /*!
     * @brief This function returns global symbol scope.
     *
     * @return SymbolScope & Global symbol scope.
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
     * @param[in] codec Used codec type in eRPC application.
     */
    void setProgramInfo(const std::string &fileName, const std::string &outputFilePath, codec_t codec);

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
    const std::string getProgramName() { return getProgramSymbol()->getName(); }

    /*!
     * @brief This function returns output directory path.
     *
     * @return Return output directory path.
     */
    std::filesystem::path getOutputDirectory() { return m_outputDirectory; }

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
    Program *getProgramSymbol();

    /*!
     * @brief This function returns used codec type in eRPC application.
     *
     * This information can bring more optimized code into generated output files.
     *
     * @retval kNotSpecified No optimization.
     * @retval kBasicCodec BasicCodec type optimization.
     */
    codec_t getCodecType() { return m_codec; }

    /*!
     * @brief This function returns crc16 of all used IDL files.
     *
     * @return Crc16 of all used IDL files.
     */
    uint16_t getIdlCrc16() { return m_idlCrc16; }

private:
    /* Instance Variables */
    AstNode *m_ast;                          /*!< Root of AstNode tree. */
    SymbolScope m_globals;                   /*!< Symbol scope data. */
    Program *m_program;                      /*!< Program symbol.*/
    std::string m_programName;               /*!< Program name set via parsed file. */
    std::string m_outputFilename;            /*!< Output file name. */
    std::filesystem::path m_outputDirectory; /*!< Output file path. */
    codec_t m_codec;                         /*!< Used codec type. */
    uint16_t m_idlCrc16;                     /*!< Crc16 of IDL files. */

    /* Private Functions */
    /*!
     * @brief This function add builtin types into symbol scope variable m_globals.
     */
    void createBuiltinTypes();
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__INTERFACEDEFINITION_H_
