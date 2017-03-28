/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
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

/*******************************************************************************
 * Variables
 ******************************************************************************/

%option c++
/* %option prefix="erpcgen" */
%option yylineno
%option never-interactive
%option yyclass="ErpcLexer"
%option noyywrap

/*******************************************************************************
 * Code
 ******************************************************************************/

%{
#include "ErpcLexer.h"
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string>
#include "HexValues.h"
#include "Value.h"

using namespace erpcgen;

/*! Always executed before all other actions when a token is matched.
    This action just assign the first and last lines of the token to
    the current line. In most cases this is correct. */
#define YY_USER_ACTION  do {                                    \
                            m_location.m_firstLine = m_currentFileInfo->m_line;          \
                            m_location.m_firstChar = m_currentFileInfo->m_column + 1;    \
                            m_currentFileInfo->m_column += yyleng;                       \
                            m_currentFileInfo->m_line = yylineno;                        \
                            m_location.m_lastLine = m_currentFileInfo->m_line;           \
                            m_location.m_lastChar = m_currentFileInfo->m_column;         \
                        } while (0);

%}

DIGIT           [0-9]
HEXDIGIT        [0-9a-fA-F]
BINDIGIT        [0-1]
EXP             ([eE][-+]?[0-9]+)
INT             ({DIGIT}+|0x{HEXDIGIT}+|0b{BINDIGIT}+)([uU]?[lL]?[lL]?)
/*FLOAT           ([+-]?[0-9]*(\.[0-9]+)?([eE][+-]?[0-9]+)?) */
FLOAT           [-+]?((([0-9]*\.[0-9]+)|([0-9]+\.)){EXP}?|[0-9]+{EXP})
IDENT           [a-zA-Z_][a-zA-Z0-9_]*
ESC             \\(x{HEXDIGIT}{2}|.)

/* start conditions */
%x doxyilcmt
%x doxymlcmt
%x mlcmt

%%

program         { return TOK_PROGRAM; }
const           { return TOK_CONST; }
import          { return TOK_IMPORT; }
enum            { return TOK_ENUM; }
struct          { return TOK_STRUCT; }
union           { return TOK_UNION; }
switch          { return TOK_SWITCH; }
case            { return TOK_CASE; }
default         { return TOK_DEFAULT; }
optional        { return TOK_OPTIONAL; }
byref           { return TOK_BYREF; }
type            { return TOK_TYPE; }
interface       { return TOK_INTERFACE; }
version         { return TOK_VERSION; }
in              { return TOK_IN; }
out             { return TOK_OUT; }
inout           { return TOK_INOUT; }
async           { return TOK_ASYNC; }
oneway          { return TOK_ONEWAY; }
list            { return TOK_LIST; }
ref             { return TOK_REF; }
true            { return TOK_TRUE; }
false           { return TOK_FALSE; }
void            { return TOK_VOID; }

{FLOAT}                             {
                                        // generate a token with a FloatValue
                                        m_value = new FloatValue(strtod(yytext, NULL));
                                        return TOK_FLOAT_LITERAL;
                                    }

{IDENT}                             {
                                        m_value = new StringValue(yytext);
                                        return TOK_IDENT;
                                    }

{INT}                               {
                                        int base = 0;
                                        uint32_t value;

                                        // check for binary number
                                        if (yytext[0] == '0' && yytext[1] == 'b')
                                        {
                                            base = 2;       // this is a binary number
                                            yytext += 2;    // skip over the "0b"
                                        }

                                        // convert value
                                        value = (uint32_t)strtoul(yytext, NULL, base);

                                        // set resulting symbol value
                                        m_value = new IntegerValue(value);
                                        return TOK_INT_LITERAL;
                                    }

[ \t]*\/\/[!\/]<.*                  {/* track trailing in-line comment if it is doxygen comment. */
                                        m_value = new StringValue(yytext);
                                        return TOK_IL_COMMENT;
                                    }

[ \t]*\/\*[\*!]<[ \t]*              { /* start trailing in-line comment if it is doxygen comment. */
                                        BEGIN(doxyilcmt);
                                        m_value = new StringValue(yytext);
                                        return TOK_IL_COMMENT;
                                    }

<doxyilcmt>\*\/                     {   /* end trailing in-line comment comment */
                                        m_value = new StringValue(yytext);
                                        BEGIN(INITIAL);
                                        return TOK_IL_COMMENT;
                                    }

<doxyilcmt>(.|\r\n|\r|\n)[ \t]*     {   /* body trailing in-line comment comment */
                                        m_value = new StringValue(yytext);
                                        return TOK_IL_COMMENT;
                                    }

"//"[!\/][^<\r\n]?.*                {
                                        m_value = new StringValue("\n" + std::string(yytext));
                                        return TOK_ML_COMMENT;
                                    }

[ \t]*"/*"[!\*][^<\r\n]?[ \t]*      { /* start comment */
                                        m_indents = 0;
                                        while (yytext[m_indents] == ' ' || yytext[m_indents] == '\t')
                                        {
                                            ++m_indents;
                                        }
                                        m_value = new StringValue("\n" + std::string(yytext + m_indents));
                                        BEGIN(doxymlcmt);
                                        return TOK_ML_COMMENT;
                                    }

<doxymlcmt>"*/"                     {   /* end comment */
                                        m_value = new StringValue(yytext);
                                        BEGIN(INITIAL);
                                        return TOK_ML_COMMENT;
                                    }

<doxymlcmt>(\r\n|\r|\n)[ \t]*       {   /* can't found easiest way to do this. On erpc file is comment for function with ident. In output are function without indent. */
                                        m_currentFileInfo->m_column = 0;
                                        std::string output;
                                        output += yytext[0];    // get \r or \n as first char
                                        ++yytext;
                                        --yyleng;
                                        if (yyleng >= 1)
                                        {
                                            if (yytext[0] == '\n')    // if \n as second char
                                            {
                                                output += yytext[0];
                                                --yyleng;
                                                ++yytext;
                                            }
                                        }

                                        if (yyleng > m_indents)    // remove indent spaces
                                        {
                                            yytext += m_indents;
                                            yyleng -= m_indents;
                                            output += yytext;
                                            m_currentFileInfo->m_column = yyleng;
                                        }
                                        else
                                        {
                                            m_currentFileInfo->m_column = 0;
                                        }

                                        m_value = new StringValue(output);
                                        return TOK_ML_COMMENT;
                                    }

<doxymlcmt>.                        {   /* body comment */
                                        m_value = new StringValue(yytext);
                                        return TOK_ML_COMMENT;
                                    }

"<<"                                { return TOK_LSHIFT; }

">>"                                { return TOK_RSHIFT; }

"->"                                { return TOK_ARROW; }

[@{}()\[\]<>=,;:+\-*/%^~&|]         { return yytext[0]; }

\"(ESC|[^\"])*\"                    {
                                        // get rid of quotes
                                        yytext++;
                                        yytext[strlen(yytext) - 1] = 0;
                                        processStringEscapes(yytext, yytext);
                                        m_value = new StringValue(yytext);
                                        return TOK_STRING_LITERAL;
                                    }

(#.*|"//"[^\/!\r\n]?.*)             /* absorb single-line comment */

"/*"[^!\*]?                         {   /* absorb multiline-line comment */
                                        BEGIN(mlcmt);
                                    }

<mlcmt>"*/"                         {   /* absorb multiline-line comment */
                                        BEGIN(INITIAL);
                                    }

<mlcmt>.                            {   /* absorb multiline-line comment */    }

<*>[ \t]                            /* eat up whitespace in all states */

<*>(\r\n|\r|\n)                     {
                                        /* eat up whitespace and count lines in all states */
                                        m_currentFileInfo->m_column = 0;
                                    }

<<EOF>>                             {
                                        if (m_currentFileInfo->m_previous != NULL)
                                        {
                                            popFile();
                                        }
                                        else
                                        {
                                            return END;
                                        }
                                    }

<*>.                                {
                                        /* all other chars produce errors */
                                        char msg[50];
                                        sprintf(msg, "unexpected character '%c' on line %d", yytext[0], yylineno);
                                        LexerError(msg);
                                    }

%%

// verbatim code copied to the bottom of the output
