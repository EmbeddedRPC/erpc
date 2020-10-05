/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// This header just wraps the standard flex C++ header to make it easier to include
// without having to worry about redefinitions of the class name every time.

#ifndef _EMBEDDED_RPC__ERPCLEXER_H_
#define _EMBEDDED_RPC__ERPCLEXER_H_

#undef yyFlexLexer
#include "AstNode.h"
#include "ParseErrors.h"
#include <FlexLexer.h>
#include <fstream>
#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

#ifndef YY_BUF_SIZE
#define YY_BUF_SIZE 16384
#endif

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpcgen {

class ErpcLexer;
}
#include "erpcgen_parser.tab.hpp"

using namespace std;

namespace erpcgen {

/*!
 * @brief This class contains necessary information about analyzed file.
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
     * @param[in] savedFile Pointer to analyzed file.
     * @param[in] fileName Name of analyzed file.
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
     * @brief This function returns crc16 of all used IDL files.
     *
     * @return Crc16 of all used IDL files.
     */
    uint16_t getIdlCrc16() { return m_idlCrc16; }

protected:
    Value *m_value;                     /*!< Value for the current token. */
    token_loc_t m_location;             /*!< Location for the current token. */
    uint32_t m_indents;                 /*!< How much indents can be removed from newlines in doxygen comments. */
    CurrentFileInfo *m_currentFileInfo; /*!< Pointer to current file info. */
    uint16_t m_idlCrc16;                /*!< Crc16 of IDL files. */

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
};

} // namespace erpcgen

#endif // _EMBEDDED_RPC__ERPCLEXER_H_
