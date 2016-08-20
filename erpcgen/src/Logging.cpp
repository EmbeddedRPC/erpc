/*
 * Copyright (c) 2013-14, Freescale Semiconductor, Inc.
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

#include "Logging.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

// init global logger to null
Logger *Log::s_logger = NULL;

void Logger::log(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    log(m_level, fmt, args);
    va_end(args);
}

void Logger::log(log_level_t level, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    log(level, fmt, args);
    va_end(args);
}

void Logger::log(const char *fmt, va_list args)
{
    log(m_level, fmt, args);
}

//! Allocates a temporary buffer which is used to hold the
//! formatted string.
void Logger::log(log_level_t level, const char *fmt, va_list args)
{
    if (level <= m_filter)
    {
        char *buffer = NULL;
        int n;
#if WIN32 || __IAR_SYSTEMS_ICC__ || __CYGWIN__ || __ARMCC_VERSION
#if WIN32
        int l = _vscprintf(fmt, args); // Does not include final NULL char.
#else
        int l = vsnprintf(NULL, 0, fmt, args);
#endif
        buffer = reinterpret_cast<char *>(malloc(l + 1));
        if (!buffer)
        {
            return;
        }
        n = vsprintf(buffer, fmt, args);
#else
        n = vasprintf(&buffer, fmt, args);
        if (!buffer)
        {
            return;
        }

#endif // WIN32 __IAR_SYSTEMS_ICC__ __CYGWIN__ __ARMCC_VERSION

        if (n > 0)
        {
            _log(level, buffer);
        }
        free(buffer);
    }
}

void Log::log(const char *fmt, ...)
{
    if (s_logger)
    {
        va_list args;
        va_start(args, fmt);
        s_logger->log(fmt, args);
        va_end(args);
    }
}

void Log::log(const string &msg)
{
    if (s_logger)
    {
        s_logger->log(msg);
    }
}

void Log::log(Logger::log_level_t level, const char *fmt, ...)
{
    if (s_logger)
    {
        va_list args;
        va_start(args, fmt);
        s_logger->log(level, fmt, args);
        va_end(args);
    }
}

void Log::log(Logger::log_level_t level, const string &msg)
{
    if (s_logger)
    {
        s_logger->log(level, msg);
    }
}

void Log::urgent(const char *fmt, ...)
{
    if (s_logger)
    {
        va_list args;
        va_start(args, fmt);
        s_logger->log(Logger::kUrgent, fmt, args);
        va_end(args);
    }
}

void Log::error(const char *fmt, ...)
{
    if (s_logger)
    {
        va_list args;
        va_start(args, fmt);
        s_logger->log(Logger::kError, fmt, args);
        va_end(args);
    }
}

void Log::warning(const char *fmt, ...)
{
    if (s_logger)
    {
        va_list args;
        va_start(args, fmt);
        s_logger->log(Logger::kWarning, fmt, args);
        va_end(args);
    }
}

void Log::info(const char *fmt, ...)
{
    if (s_logger)
    {
        va_list args;
        va_start(args, fmt);
        s_logger->log(Logger::kInfo, fmt, args);
        va_end(args);
    }
}

void Log::info2(const char *fmt, ...)
{
    if (s_logger)
    {
        va_list args;
        va_start(args, fmt);
        s_logger->log(Logger::kInfo2, fmt, args);
        va_end(args);
    }
}

void Log::debug(const char *fmt, ...)
{
    if (s_logger)
    {
        va_list args;
        va_start(args, fmt);
        s_logger->log(Logger::kDebug, fmt, args);
        va_end(args);
    }
}

void Log::debug2(const char *fmt, ...)
{
    if (s_logger)
    {
        va_list args;
        va_start(args, fmt);
        s_logger->log(Logger::kDebug2, fmt, args);
        va_end(args);
    }
}

void StdoutLogger::_log(Logger::log_level_t level, const char *msg)
{
    if (level <= m_stderrLevel)
    {
        fprintf(stderr, "%s", msg);
    }
    else
    {
        fprintf(stdout, "%s", msg);
    }
}
