/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
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

// This header just wraps the standard flex C++ header to make it easier to include
// without having to worry about redefinitions of the class name every time.

#ifndef _EMBEDDED_RPC__ERPCLEXER_H_
#define _EMBEDDED_RPC__ERPCLEXER_H_

#include "AstNode.h"
#undef yyFlexLexer
#include <FlexLexer.h>
#include "ParseErrors.h"
#include <vector>
#include <string>
#include <fstream>

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

#ifndef YY_BUF_SIZE
#define YY_BUF_SIZE 16384
#endif

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen
{
class ErpcLexer;
}
#include "erpcgen_parser.tab.hpp"

using namespace std;

namespace erpcgen
{
/*!
 * @brief This class contains necessary information about analysed file.
 */
class CurrentFileInfo
{
public:
    /*!
     * @brief Constructor.
     *
     * This function set default values to object variables. Pointer to file ifstream,
     * file name and current folder path are given as function parameters.
     *
     * @param[in] savedFile Pointer to analysed file.
     * @param[in] fileName Name of analysed file.
     * @param[in] currentFolderPath Path to folder of current file.
     */
    CurrentFileInfo(std::ifstream *savedFile, std::string fileName, std::string currentFolderPath)
    : m_savedFile(savedFile)
    , m_previous(NULL)
    , m_line(1)
    , m_column(0)
    , m_fileName(fileName)
    , m_currentFolderPath(currentFolderPath)
    {
    }

    /*!
     * @brief Destructor.
     *
     * This function will close used file, when destructor is called.
     */
    ~CurrentFileInfo()
    {
        m_savedFile->close();
        m_savedFile.safe_delete();
    }

    smart_ptr<std::ifstream> m_savedFile; /*!< Pointer to current file */
    CurrentFileInfo *m_previous;          /*!< Pointer to previous saved file info. */
    int m_line;                           /*!< Line number in current file. */
    int m_column;                         /*!< Column number on the line in current file. */
    std::string m_fileName;               /*!< File name for current file. */
    std::string m_currentFolderPath;      /*!< Path to folder of current file. */
};

/*!
 * @brief Lexical scanner class for erpcgen interface files.
 *
 * This class is a subclass of the standard C++ lexer class produced by
 * Flex. It's primary purpose is to provide a clean way to report values
 * for symbols, without using the yylval global. This is necessary because
 * the parser produced by Bison is a "pure" parser.
 */
class ErpcLexer : public yyFlexLexer
{
public:
    /*!
     * @brief This function is constructor of ErpcLexer class.
     *
     * This function set given file for flex lexer.
     *
     * @param[in] inputFile This variable contains file name and can contains also path to file. File is given to flex
     * lexical analysis.
     */
    ErpcLexer(const char *inputFile);

    /*!
     * @brief This function is destructor of ErpcLexer class.
     */
    ~ErpcLexer();

    /*!
     * @brief This function returns next Token from flex lexer.
     *
     *@return A newly allocated Token object. Ownership of this token is passed to the caller,
     *     who is responsible for its deallocation. If the end of input is reached, NULL is
     *     returned instead.
     */
    virtual Token *getNextToken();

    /*!
     * @brief returns the current token's location in \a loc.
     *
     *@return Current token's location.
     */
    inline token_loc_t &getLocation() { return m_location; }
    /*!
     * @brief Suspend actual and give new file to flex.
     *
     * This function set to flex lexer new file, when import file request is inside parsed file.
     *
     * @param[in] fileName File name, which can contains also path to file, which is given to flex lexical analysis.
     */
    void pushFile(const std::string &fileName);

    /*!
     * @brief Give suspended file to flex.
     */
    void popFile();

    /*!
     * @brief return current file name
     *
     * @return Current file name.
     */
    inline std::string &getFileName() { return m_currentFileInfo->m_fileName; }
    /*!
     * @brief This function open file.
     *
     * This function will try find and open file, which is given be file name
     * variable. When file is found and opened new current file info object is
     * created.
     *
     * @param[in] fileName File name, which can contains also path to file,
     *          which is given to flex lexical analysis.
     *
     * @return return new current file info object if file is
     *          found and opened.
     *
     * @exception std::runtime_error Thrown if file is not found.
     * @exception std::runtime_error Thrown if file can not open.
     * @exception std::runtime_error Thrown if can not create ifstream object from file.
     */
    CurrentFileInfo *openFile(const std::string &fileName);

protected:
    Value *m_value;                     /*!< Value for the current token. */
    token_loc_t m_location;             /*!< Location for the current token. */
    CurrentFileInfo *m_currentFileInfo; /*!< Pointer to current file info. */
    uint32_t m_indents;                 /*!< How much indents can be removed from newlines in doxygen comments. */

    /*!
     * @brief This function thrown lexical_error with given message.
     *
     * @param[in] msg Message, which is showed, when this functions is called.
     *
     * @exception lexical_error Thrown always, when function is called.
     */
    virtual void LexerError(const char *msg);

    /*!
     * @brief Reads the \a in string and writes to the \a out string.
     *
     * These strings can be the same string since the read head is always in front of the write head.
     *
     * @param[in] in Input string containing C-style escape sequences.
     * @param[out] out Output string. All escape sequences in the input string have been converted
     *      to the actual characters. May point to the same string as \a in.
     *
     * @return The length of the resulting \a out string. This length is necessary because
     *      the string may have contained escape sequences that inserted null characters.
     */
    int processStringEscapes(const char *in, char *out);

    /*!
     * @brief Internal lexer interface.
     *
     *@return returns one token.
     */
    virtual int yylex();
};

}; // namespace erpcgen

#endif // _EMBEDDED_RPC__ERPCLEXER_H_
