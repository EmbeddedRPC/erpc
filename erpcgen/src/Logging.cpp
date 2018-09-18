/*
 * Copyright (c) 2013-14, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "Logging.h"
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

using namespace std;

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
