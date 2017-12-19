/*
 * The Clear BSD License
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 * that the following conditions are met:
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
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
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
#include "ErpcLexer.h"
#include "erpc_version.h"
#include "Generator.h"
#include "HexValues.h"
#include "SearchPath.h"
#include "crc16.h"
#include <algorithm>
#include <fstream>
#include <streambuf>
#include <string>

using namespace erpcgen;
using namespace std;

#if __WIN32__
#define PATH_SEP_CHAR '\\'
#else
#define PATH_SEP_CHAR '/'
#endif

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

ErpcLexer::ErpcLexer(const char *inputFile)
: m_value(nullptr)
, m_indents(0)
, m_currentFileInfo(NULL)
, m_idlCrc16(0)
{
    m_currentFileInfo = openFile(inputFile);
    yyrestart(m_currentFileInfo->m_savedFile.get()); // instead of yyFlexLexer(idlFile);
}

ErpcLexer::~ErpcLexer()
{
    if (m_currentFileInfo)
    {
        delete m_currentFileInfo;
    }
}

Token *ErpcLexer::getNextToken()
{
    // Ensure previous value is cleared before calling into lexer.
    m_value = nullptr;

    // Run lexer.
    int tokenValue = yylex();

    Token *tok = nullptr;
    if (tokenValue)
    {
        tok = new Token(tokenValue, m_value, m_location);
    }
    return tok;
}

void ErpcLexer::LexerError(const char *msg)
{
    throw lexical_error(msg);
}

int ErpcLexer::processStringEscapes(const char *in, char *out)
{
    int count = 0;
    while (*in)
    {
        switch (*in)
        {
            case '\\':
            {
                // start of an escape sequence
                char c = *++in;
                switch (c)
                {
                    case 0: // end of the string, bail
                        break;
                    case 'x':
                    {
                        // start of a hex char escape sequence

                        // read high and low nibbles, checking for end of string
                        char hi = *++in;
                        if (hi == 0)
                            break;
                        char lo = *++in;
                        if (lo == 0)
                            break;

                        if (isHexDigit(hi) && isHexDigit(lo))
                        {
                            *out++ = (hexCharToInt(hi) << 4) | hexCharToInt(lo);
                            count++;
                        }
                        else
                        {
                            // not hex digits, the \x must have wanted an 'x' char
                            *out++ = 'x';
                            *out++ = hi;
                            *out++ = lo;
                            count += 3;
                        }
                        break;
                    }
                    case 'n':
                        *out++ = '\n';
                        count++;
                        break;
                    case 't':
                        *out++ = '\t';
                        count++;
                        break;
                    case 'r':
                        *out++ = '\r';
                        count++;
                        break;
                    case 'b':
                        *out++ = '\b';
                        count++;
                        break;
                    case 'f':
                        *out++ = '\f';
                        count++;
                        break;
                    case '0':
                        *out++ = '\0';
                        count++;
                        break;
                    default:
                        *out++ = c;
                        count++;
                        break;
                }
                break;
            }

            default:
                // copy all other chars directly
                *out++ = *in++;
                count++;
        }
    }

    // place terminating null char on output
    *out = 0;
    return count;
}

void ErpcLexer::pushFile(const string &fileName)
{
    CurrentFileInfo *newSavedFileInfo = openFile(fileName);

    // change current file info to new file info
    newSavedFileInfo->m_previous = m_currentFileInfo;
    m_currentFileInfo = newSavedFileInfo;

    // Reset current line number.
    yylineno = 1;

    // give to flex new buffer (given file)
    yypush_buffer_state(yy_create_buffer(newSavedFileInfo->m_savedFile.get(), YY_BUF_SIZE));
}

void ErpcLexer::popFile()
{
    // set variables suspended file
    CurrentFileInfo *holdPointerToCurrentFile = m_currentFileInfo;
    m_currentFileInfo = m_currentFileInfo->m_previous;
    delete holdPointerToCurrentFile;
    PathSearcher::getGlobalSearcher().setTempPath(m_currentFileInfo->m_currentFolderPath);

    // Restore current line number.
    yylineno = m_currentFileInfo->m_line;

    // give to flex before this buffer (suspended file)
    yypop_buffer_state();
}

CurrentFileInfo *ErpcLexer::openFile(const string &fileName)
{
    // search file in path
    string foundFile, currentFolderPath;
    if (!PathSearcher::getGlobalSearcher().search(fileName, PathSearcher::target_type_t::kFindFile, true, foundFile))
    {
        throw runtime_error(format_string("could not find input file %s in defined directories", fileName.c_str()));
    }

    if (fileName.rfind(PATH_SEP_CHAR) != string::npos)
    {
        int fileSepPos = foundFile.rfind(PATH_SEP_CHAR);
        currentFolderPath = foundFile.substr(0, fileSepPos);
        PathSearcher::getGlobalSearcher().setTempPath(currentFolderPath);
    }
    else
    {
        if (m_currentFileInfo != NULL)
        {
            currentFolderPath = m_currentFileInfo->m_currentFolderPath;
        }
        else
        {
            currentFolderPath = "";
        }
    }
    // open file
    ifstream *inputFile = new ifstream(foundFile.c_str(), ios_base::in | ios_base::binary);
    if (inputFile)
    {
        if (!inputFile->is_open())
        {
            delete inputFile;
            throw runtime_error(format_string("could not open input file %s", foundFile.c_str()));
        }
    }
    else
    {
        throw runtime_error(format_string("could not create ifstream object from file %s", foundFile.c_str()));
    }

    /* Counting CRC16 for Generator. */
    string str((istreambuf_iterator<char>(*inputFile)), istreambuf_iterator<char>());
    erpc::Crc16 crc16 = erpc::Crc16(ERPC_VERSION_NUMBER);
    m_idlCrc16 += crc16.computeCRC16((const uint8_t *)str.c_str(), str.size());

    /* Reset state to beginning of file. */
    inputFile->clear();
    inputFile->seekg(0, ios::beg);

    return new CurrentFileInfo(inputFile, foundFile, currentFolderPath);
}
