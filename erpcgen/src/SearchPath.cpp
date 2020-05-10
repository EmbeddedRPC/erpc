/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "SearchPath.h"
#include <cstdio>

#if __WIN32__
#define PATH_SEP_CHAR '\\'
#define PATH_SEP_STRING "\\"
#else
#define PATH_SEP_CHAR '/'
#define PATH_SEP_STRING "/"
#endif

PathSearcher *PathSearcher::s_searcher = NULL;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

//! This function will create the global path search object if it has
//! not already been created.
PathSearcher &PathSearcher::getGlobalSearcher()
{
    if (!s_searcher)
    {
        s_searcher = new PathSearcher;
    }

    return *s_searcher;
}

void PathSearcher::addSearchPath(const std::string &path)
{
    m_paths.push_back(path);
}

void PathSearcher::setTempPath(const std::string &path)
{
    m_tempPath = path;
}

//! The \a base path argument can be either a relative or absolute path. If the path
//! is relative, then it is joined with search paths one after another until a matching
//! file is located or all search paths are exhausted. If the \a base is absolute,
//! only that path is tested and if invalid false is returned.
//!
//! \param base A path to the file that is to be found.
//! \param targetType Currently ignored. In the future it will let you select whether to
//!		find a file or directory.
//! \param searchCwd If set to true, the current working directory is searched before using
//!		any of the search paths. Otherwise only the search paths are considered.
//! \param[out] result When true is returned this string is set to the first path at which
//!		a valid file was found.
//!
//! \retval true A matching file was found among the search paths. The contents of \a result
//!		are a valid path.
//! \retval false No match could be made. \a result has been left unmodified.
bool PathSearcher::search(const std::string &base, target_type_t targetType, bool searchCwd, std::string &result)
{
    FILE *tempFile;
    bool absolute = isAbsolute(base);

    // Try cwd first if requested. Same process applies to absolute paths.
    if (absolute || searchCwd)
    {
        tempFile = fopen(base.c_str(), "r");
        if (tempFile)
        {
            fclose(tempFile);
            result = base;
            return true;
        }
    }

    // If the base path is absolute and the previous test failed, then we don't go any further.
    if (absolute)
    {
        return false;
    }

    // Set temporary path to list of paths if path is not empty string.
    if (m_tempPath != "")
    {
        m_paths.push_front(m_tempPath);
    }

    bool foundFile = false;
    // Iterate over all search paths.
    string_list_t::const_iterator it = m_paths.begin();
    for (; it != m_paths.end(); ++it)
    {
        std::string searchPath = joinPaths(*it, base);

        tempFile = fopen(searchPath.c_str(), "r");
        if (tempFile)
        {
            fclose(tempFile);
            result = searchPath;
            foundFile = true;
            break;
        }
    }

    // Remove temporary path from list of paths if path is not empty.
    if (m_tempPath != "")
    {
        m_paths.pop_front();
    }

    return foundFile;
}

bool PathSearcher::isAbsolute(const std::string &path)
{
#if __WIN32__
    return path.size() >= 3 && path[1] == ':' && path[2] == PATH_SEP_CHAR;
#else
    return path.size() >= 1 && path[0] == PATH_SEP_CHAR;
#endif
}

std::string PathSearcher::joinPaths(const std::string &first, const std::string &second)
{
    // Start with first string.
    std::string result = first;

    // Add path separator if needed
    if ((first[first.size() - 1] != PATH_SEP_CHAR) && (second[0] != PATH_SEP_CHAR))
    {
        result += PATH_SEP_STRING;
    }

    // Append the second string.
    result += second;

    // And return the whole mess.
    return result;
}
