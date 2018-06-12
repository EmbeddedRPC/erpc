/*
 * The Clear BSD License
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 *  that the following conditions are met:
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
#if !defined(_searchpath_h_)
#define _searchpath_h_

#include <list>
#include <string>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief Handles searching a list of paths for a file.
 */
class PathSearcher
{
public:
    //!
    enum _target_type
    {
        kFindFile,
        kFindDirectory
    };

    //!
    typedef enum _target_type target_type_t;

protected:
    //! Global search object singleton.
    static PathSearcher *s_searcher;

public:
    //! \brief Access global path searching object.
    static PathSearcher &getGlobalSearcher();

public:
    //! \brief Constructor.
    PathSearcher() {}

    //! \brief Add a new search path to the end of the list.
    void addSearchPath(const std::string &path);

    //! \brief Set temporary path.
    void setTempPath(const std::string &path);

    //! \brief Attempts to locate a file by using the search paths.
    bool search(const std::string &base, target_type_t targetType, bool searchCwd, std::string &result);

protected:
    typedef std::list<std::string> string_list_t; //!< Linked list of strings.
    string_list_t m_paths;                        //!< Ordered list of paths to search.
    std::string m_tempPath;                       //!< Possibility to add temporary path.

    //! \brief Returns whether \a path is absolute.
    bool isAbsolute(const std::string &path);

    //! \brief Combines two paths into a single one.
    std::string joinPaths(const std::string &first, const std::string &second);
};

#endif // _searchpath_h_
