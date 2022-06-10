/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
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
    }; /*!< Type of searched item. */

    //!
    typedef enum _target_type target_type_t; /*!< Type of searched item. */

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
