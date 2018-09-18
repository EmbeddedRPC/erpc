/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "format_string.h"
#include "smart_ptr.h"
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>

//! Size of the temporary buffer to hold the formatted output string.
#define WIN32_FMT_BUF_LEN (2048)

//! The purpose of this function to provide a convenient way of generating formatted
//! STL strings inline. This is especially useful when throwing exceptions that take
//! a std::string for a message. The length of the formatted output string is limited
//! only by memory. Memory temporarily allocated for the output string is disposed of
//! before returning.
//!
//! For Win32, the maximum output size is 2048 characters. For POSIX systems, there
//! is no fixed maximum because the vasprintf() function that allocates exactly
//! the required storage is available.
//!
//! Example usage:
//! \code
//!     throw std::runtime_error(format_string("error on line %d", line));
//! \endcode
//!
//! \param fmt Format string using printf-style format markers.
//! \return An STL string object of the formatted output.
std::string format_string(const char *fmt, ...)
{
    char *buf = 0;
    va_list vargs;
    va_start(vargs, fmt);
    int result;
#if WIN32 || __CYGWIN__
    buf = (char *)malloc(WIN32_FMT_BUF_LEN);
    if (buf)
    {
#if WIN32
        result = _vsnprintf(buf, WIN32_FMT_BUF_LEN, fmt, vargs);
#else  //__CYGWIN__
        result = vsnprintf(buf, WIN32_FMT_BUF_LEN, fmt, vargs);
#endif // WIN32
    }
    else
    {
        result = -1;
    }
#else
    result = vasprintf(&buf, fmt, vargs);
#endif // WIN32 __CYGWIN__
    va_end(vargs);
    if (result != -1 && buf)
    {
        smart_free_ptr<char> freebuf(buf);
        return std::string(buf);
    }
    return "";
}
