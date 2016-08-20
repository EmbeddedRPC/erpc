/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
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

#include "format_string.h"
#include "smart_ptr.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdexcept>
#include <stdlib.h>

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
//!		throw std::runtime_error(format_string("error on line %d", line));
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
#endif // WIN32 __CYGWIN__
    }
#else
    result = vasprintf(&buf, fmt, vargs);
#endif // WIN32
    va_end(vargs);
    if (result != -1 && buf)
    {
        smart_free_ptr<char> freebuf(buf);
        return std::string(buf);
    }
    return "";
}
