/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
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
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

InterfaceDefinition::InterfaceDefinition()
: m_ast(nullptr)
, m_globals()
, m_program(nullptr)
, m_programName("")
, m_outputFilename("")
, m_codec(kNotSpecified)
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
    m_globals.addSymbol(new BuiltinType("ustring", BuiltinType::_builtin_type::kUStringType));
    m_globals.addSymbol(new BuiltinType("binary", BuiltinType::_builtin_type::kBinaryType));
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
