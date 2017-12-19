/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
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

#include "InterfaceDefinition.h"
#include "AstNode.h"
#include "AstWalker.h"
#include "ErpcLexer.h"
#include "Logging.h"
#include "SymbolScanner.h"
#include "annotations.h"
#include "erpcgen_parser.tab.hpp"
#include "types/AliasType.h"
#include "types/BuiltinType.h"
#include "types/StructType.h"

using namespace erpcgen;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

InterfaceDefinition::InterfaceDefinition()
: m_ast(nullptr)
, m_globals()
, m_programName("")
, m_outputFilename("")
, m_error_handling_check(kAll)
, m_codec(kNotSpecified)
{
    init();
}

void InterfaceDefinition::init()
{
    createBuiltinTypes();
}

uint16_t InterfaceDefinition::parse(const char *inputFile)
{
    // create lexer instance
    ErpcLexer lexer(inputFile);
    int result = yyparse(&lexer, &m_ast);

    // check results
    if (result || !m_ast)
    {
        throw std::runtime_error("failed to parse command file");
    }

    //    Log::info("Parsing was successful!\n");

    // dump AST
    AstPrinter printer(m_ast);
    printer.dispatch();

    // Build table of symbols.
    SymbolScanner scanner(&m_globals, std::string(inputFile));
    scanner.startWalk(m_ast);

    m_globals.dump();

    return lexer.getIdlCrc16();
}

void InterfaceDefinition::createBuiltinTypes()
{
    m_globals.addSymbol(new BuiltinType("bool", BuiltinType::_builtin_type::kBoolType));
    m_globals.addSymbol(new BuiltinType("int8", BuiltinType::_builtin_type::kInt8Type));
    m_globals.addSymbol(new BuiltinType("int16", BuiltinType::_builtin_type::kInt16Type));
    m_globals.addSymbol(new BuiltinType("int32", BuiltinType::_builtin_type::kInt32Type));
    m_globals.addSymbol(new BuiltinType("int64", BuiltinType::_builtin_type::kInt64Type));
    m_globals.addSymbol(new BuiltinType("uint8", BuiltinType::_builtin_type::kUInt8Type));
    m_globals.addSymbol(new BuiltinType("uint16", BuiltinType::_builtin_type::kUInt16Type));
    m_globals.addSymbol(new BuiltinType("uint32", BuiltinType::_builtin_type::kUInt32Type));
    m_globals.addSymbol(new BuiltinType("uint64", BuiltinType::_builtin_type::kUInt64Type));
    m_globals.addSymbol(new BuiltinType("float", BuiltinType::_builtin_type::kFloatType));
    m_globals.addSymbol(new BuiltinType("double", BuiltinType::_builtin_type::kDoubleType));
    m_globals.addSymbol(new BuiltinType("string", BuiltinType::_builtin_type::kStringType));
    m_globals.addSymbol(new BuiltinType("binary", BuiltinType::_builtin_type::kBinaryType));
}

void InterfaceDefinition::setProgramInfo(const std::string &filename, const std::string &outputDir, codec_t codec)
{
    setOutputFilename(filename);
    setOutputDirectory(outputDir);
    setErrorHandlingChecksType();
    m_codec = codec;
}

bool InterfaceDefinition::hasProgramSymbol()
{
    return 1 == m_globals.getSymbolsOfType(Symbol::kProgramSymbol).size();
}

Program *InterfaceDefinition::getProgramSymbol()
{
    if (1 != m_globals.getSymbolsOfType(Symbol::kProgramSymbol).size())
    {
        return nullptr;
    }
    else
    {
        assert(nullptr != dynamic_cast<Program *>(m_globals.getSymbolsOfType(Symbol::kProgramSymbol)[0]));
        return dynamic_cast<Program *>(m_globals.getSymbolsOfType(Symbol::kProgramSymbol)[0]);
    }
}

void InterfaceDefinition::setOutputDirectory(const std::string &outputDir)
{
    m_outputDirectory = outputDir;

    if (hasProgramSymbol())
    {
        std::string outputDir = getProgramSymbol()->getAnnStringValue(OUTPUT_DIR_ANNOTATION);
        if (!outputDir.empty())
        {
            m_outputDirectory /= outputDir;
        }
    }
}

void InterfaceDefinition::setOutputFilename(const std::string &filename)
{
    if (hasProgramSymbol())
    {
        m_outputFilename = getProgramSymbol()->getName();
    }
    else
    {
        m_outputFilename = filename;
    }
}

void InterfaceDefinition::setErrorHandlingChecksType()
{
    if (hasProgramSymbol())
    {
        if (getProgramSymbol()->findAnnotation(NO_ALLOC_ERRORS_ANNOTATION))
        {
            m_error_handling_check = (_error_handling_checks)(((int)m_error_handling_check) + 1);
        }
        if (getProgramSymbol()->findAnnotation(NO_INFRA_ERRORS_ANNOTATION))
        {
            m_error_handling_check = (_error_handling_checks)(((int)m_error_handling_check) + 2);
        }
    }
}
