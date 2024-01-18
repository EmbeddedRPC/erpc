/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "InterfaceDefinition.hpp"

#include "AstNode.hpp"
#include "AstWalker.hpp"
#include "ErpcLexer.hpp"
#include "Logging.hpp"
#include "SymbolScanner.hpp"
#include "annotations.h"
#include "erpcgen_parser.tab.hpp"
#include "types/AliasType.hpp"
#include "types/BuiltinType.hpp"
#include "types/StructType.hpp"

using namespace erpcgen;
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

InterfaceDefinition::InterfaceDefinition() :
m_ast(nullptr), m_globals(), m_program(nullptr), m_programName(""), m_outputFilename(""),
m_codec(codec_t::kNotSpecified), m_idlCrc16(0)
{
    init();
}

void InterfaceDefinition::init()
{
    createBuiltinTypes();
}

void InterfaceDefinition::parse(const char *inputFile)
{
    // create lexer instance
    ErpcLexer lexer(inputFile);
    int result = yyparse(&lexer, &m_ast);
    m_idlCrc16 = lexer.getIdlCrc16();

    // check results
    if (result || !m_ast)
    {
        throw runtime_error("failed to parse command file");
    }

    //    Log::info("Parsing was successful!\n");

    // dump AST
    AstPrinter printer(m_ast);
    printer.dispatch();

    // Build table of symbols.
    SymbolScanner scanner(&m_globals, string(inputFile));
    scanner.startWalk(m_ast);
    m_program = scanner.getProgram();

    m_globals.dump();
}

void InterfaceDefinition::createBuiltinTypes()
{
    m_globals.addSymbol(new BuiltinType("bool", BuiltinType::builtin_type_t::kBoolType));
    m_globals.addSymbol(new BuiltinType("int8", BuiltinType::builtin_type_t::kInt8Type));
    m_globals.addSymbol(new BuiltinType("int16", BuiltinType::builtin_type_t::kInt16Type));
    m_globals.addSymbol(new BuiltinType("int32", BuiltinType::builtin_type_t::kInt32Type));
    m_globals.addSymbol(new BuiltinType("int64", BuiltinType::builtin_type_t::kInt64Type));
    m_globals.addSymbol(new BuiltinType("uint8", BuiltinType::builtin_type_t::kUInt8Type));
    m_globals.addSymbol(new BuiltinType("uint16", BuiltinType::builtin_type_t::kUInt16Type));
    m_globals.addSymbol(new BuiltinType("uint32", BuiltinType::builtin_type_t::kUInt32Type));
    m_globals.addSymbol(new BuiltinType("uint64", BuiltinType::builtin_type_t::kUInt64Type));
    m_globals.addSymbol(new BuiltinType("float", BuiltinType::builtin_type_t::kFloatType));
    m_globals.addSymbol(new BuiltinType("double", BuiltinType::builtin_type_t::kDoubleType));
    m_globals.addSymbol(new BuiltinType("string", BuiltinType::builtin_type_t::kStringType));
    m_globals.addSymbol(new BuiltinType("ustring", BuiltinType::builtin_type_t::kUStringType));
    m_globals.addSymbol(new BuiltinType("binary", BuiltinType::builtin_type_t::kBinaryType));
}

void InterfaceDefinition::setProgramInfo(const string &filename, const string &outputDir, codec_t codec)
{
    setOutputFilename(filename);
    m_outputDirectory = outputDir;
    m_codec = codec;
}

bool InterfaceDefinition::hasProgramSymbol()
{
    return m_program != nullptr;
}

Program *InterfaceDefinition::getProgramSymbol()
{
    if (!hasProgramSymbol())
    {
        return nullptr;
    }
    else
    {
        return m_program;
    }
}

void InterfaceDefinition::setOutputFilename(const string &filename)
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
