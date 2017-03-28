/*
 * Copyright (c) 2013-14, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
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
 * o Neither the name of the copyright holder nor the names of its
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
#if !defined(_Logging_h_)
#define _Logging_h_

#include <cassert>
#include <cstdarg>
#include <string>

#if !(__embedded_cplusplus)
using namespace std;
#endif

/*!
 * \brief Base logger class.
 *
 * There are two types of logging levels that are used by this class. First
 * there is the filter level. Any log message that is assigned a level
 * higher than the current filter level is discarded. Secondly there is the
 * current output level. Log messages that do not have their own level
 * use the current output level to determine if they should be shown or
 * not.
 *
 * The two methods setFilterLevel() and setOutputLevel() set the filter
 * and default output logging levels, respectively. There are corresponding
 * getter methods as well. Both the filter and output levels are
 * initialized to #kInfo during object construction.
 *
 * Most use of the logger classes is expected to be through the Log
 * class. It provides static logging methods that call through to a global
 * singleton logger instance. There is also a Log::SetOutputLevel utility
 * class that makes it extremely easiy to temporarily change the default
 * output logging level.
 *
 * Of all the overloaded log() methods in this class, none of them are
 * really expected to be reimplemented by subclasses. Instead, there is
 * the single protected _log() method that takes a simple string pointer.
 * The other log methods all wind up calling _log(), so it provides a
 * single point to override. In fact, _log() is pure virtual, so subclasses
 * must implement it.
 *
 * \see Log
 */
class Logger
{
public:
    //! \brief Logging levels.
    enum log_level_t
    {
        kUrgent = 0,     //!< The lowest level, for messages that must always be logged.
        kError,          //!< For fatal error messages.
        kWarning,        //!< For non-fatal warning messages.
        kInfo,           //!< The normal log level, for status messages.
        kInfo2,          //!< For verbose status messages.
        kDebug,          //!< For internal reporting.
        kDebug2,         //!< Highest log level; verbose debug logging.
        kInfo1 = kInfo,  //!< Alias for #kInfo
        kDebug1 = kDebug //!< Alias for #kDebug
    };

public:
    //! \brief Default constructor.
    Logger()
    : m_filter(kInfo)
    , m_level(kInfo)
    {
    }

    //! \brief Destructor.
    virtual ~Logger() {}
    //! \name Logging levels
    //@{
    //! \brief Changes the logging level to \a level.
    inline void setFilterLevel(log_level_t level) { m_filter = level; }

    //! \brief Returns the current logging filter level.
    inline log_level_t getFilterLevel() const { return m_filter; }

    //! \brief Changes the logging output level to \a level.
    inline void setOutputLevel(log_level_t level) { m_level = level; }

    //! \brief Returns the current logging output level.
    inline log_level_t getOutputLevel() const { return m_level; }
    //@}

    //! \name Logging
    //@{
    //! \brief Log with format.
    virtual void log(const char *fmt, ...);

    //! \brief Log a string object.
    virtual void log(const string &msg) { log(msg.c_str()); }

    //! \brief Log with format at a specific output level.
    virtual void log(log_level_t level, const char *fmt, ...);

    //! \brief Log a string output at a specific output level.
    virtual void log(log_level_t level, const string &msg) { log(level, msg.c_str()); }
    //! \brief Log with format using an argument list.
    virtual void log(const char *fmt, va_list args);

    //! \brief Log with format using an argument with a specific output level.
    virtual void log(log_level_t level, const char *fmt, va_list args);
    //@}

protected:
    log_level_t m_filter; //!< The current logging filter level.
    log_level_t m_level;  //!< The current log output level.

protected:
    //! \brief The base pure virtual logging function implemented by subclasses.
    virtual void _log(Logger::log_level_t level, const char *msg) = 0;
};

/*!
 * \brief Wraps a set of static functions for easy global logging access.
 *
 * This class has a set of static methods that make it easy to access a global
 * logger instance without having to worry about extern symbols. It does this
 * by keeping a static member variable pointing at the singleton logger instance,
 * which is set with the setLogger() static method.
 *
 * There is also an inner utility class called SetOutputLevel that uses
 * C++ scoping rules to temporarily change the output logging level. When the
 * SetOutputLevel instance falls out of scope the output level is restored
 * to the previous value.
 */
class Log
{
public:
    //! \name Singleton logger access
    //@{
    //! \brief Returns the current global logger singleton.
    static inline Logger *getLogger() { return s_logger; }

    //! \brief Sets the global logger singleton instance.
    static inline void setLogger(Logger *logger) { s_logger = logger; }
    //@}

    //! \name Logging
    //@{
    //! \brief Log with format.
    static void log(const char *fmt, ...);

    //! \brief Log a string object.
    static void log(const string &msg);

    //! \brief Log with format at a specific output level.
    static void log(Logger::log_level_t level, const char *fmt, ...);

    //! \brief Log a string output at a specific output level.
    static void log(Logger::log_level_t level, const string &msg);
    //@}

    //! @name Logging level helpers
    //!
    //! These static methods log a message with an implicit log level.
    //@{
    static void urgent(const char *fmt, ...);  //!< Log a message with #Logger::kUrgent level.
    static void error(const char *fmt, ...);   //!< Log a message with #Logger::kError level.
    static void warning(const char *fmt, ...); //!< Log a message with #Logger::kWarning level.
    static void info(const char *fmt, ...);    //!< Log a message with #Logger::kInfo level.
    static void info2(const char *fmt, ...);   //!< Log a message with #Logger::kInfo2 level.
    static void debug(const char *fmt, ...);   //!< Log a message with #Logger::kDebug level.
    static void debug2(const char *fmt, ...);  //!< Log a message with #Logger::kDebug2 level.
    //@}

protected:
    static Logger *s_logger; //!< The single global logger instance.

public:
    /*!
     * \brief Utility class to temporarily change the logging output level.
     *
     * This class will change the current logging output level of a given
     * logger instance. Then when it falls out of scope it will set the
     * level back to what it was originally.
     *
     * Use like this:
     * \code
     *      // output level is some value here
     *      {
     *          Log::SetOutputLevel leveler(Logger::DEBUG);
     *          // now output level is DEBUG
     *          Log::log("my debug message 1");
     *          Log::log("my debug message 2");
     *      }
     *      // output level is restored to previous value
     * \endcode
     */
    class SetOutputLevel
    {
    public:
        //! \brief Default constructor.
        //!
        //! Saves the current logging output level of the global logger,
        //! as managed by the Log class, and sets the new level to \a level.
        SetOutputLevel(Logger::log_level_t level)
        : m_logger(Log::getLogger())
        , m_saved(Logger::kInfo)
        {
            assert(m_logger);
            m_saved = m_logger->getOutputLevel();
            m_logger->setOutputLevel(level);
        }

        //! \brief Constructor.
        //!
        //! Saves the current logging output level of \a logger and sets
        //! the new level to \a level.
        SetOutputLevel(Logger *logger, Logger::log_level_t level)
        : m_logger(logger)
        , m_saved(logger->getOutputLevel())
        {
            assert(m_logger);
            m_logger->setOutputLevel(level);
        }

        //! \brief Destructor.
        //!
        //! Restores the saved logging output level.
        ~SetOutputLevel() { m_logger->setOutputLevel(m_saved); }

    protected:
        Logger *m_logger;            //!< The logger instance we're controlling.
        Logger::log_level_t m_saved; //!< Original logging output level.
    };
};

/*!
 * \brief Simple logger that writes to stdout and stderr.
 */
class StdoutLogger : public Logger
{
public:
    StdoutLogger(Logger::log_level_t stderrLevel = Logger::kWarning)
    : m_stderrLevel(stderrLevel)
    {
    }

protected:
    //! Logs at and below this level get sent to stderr.
    Logger::log_level_t m_stderrLevel;

    //! \brief Logs the message to stdout.
    virtual void _log(Logger::log_level_t level, const char *msg);
};

#endif // _Logging_h_
