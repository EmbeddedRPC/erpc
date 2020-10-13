/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_version.h"

#include "CGenerator.h"
#include "ErpcLexer.h"
#include "InterfaceDefinition.h"
#include "Logging.h"
#include "PythonGenerator.h"
#include "SearchPath.h"
#include "UniqueIdChecker.h"
#include "options.h"
#include "types/Program.h"

#include <cstdint>
#include <cstdlib>
#include <stdexcept>
#include <vector>

/*!
 * @brief Entry point for the tool.
 */
int main(int argc, char *argv[], char *envp[]);

namespace erpcgen {
using namespace std;
////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

/*! The tool's name. */
const char k_toolName[] = "erpcgen";

/*! Current version number for the tool. */
const char k_version[] = ERPC_VERSION;

/*! Copyright string. */
const char k_copyright[] = "Copyright 2016-2020 NXP. All rights reserved.";

static const char *k_optionsDefinition[] = { "?|help",
                                             "V|version",
                                             "o:output <filePath>",
                                             "v|verbose",
                                             "I:path <filePath>",
                                             "g:generate <language>",
                                             "c:codec <codecType>",
                                             NULL };

/*! Help string. */
const char k_usageText[] =
    "\nOptions:\n\
  -?/--help                    Show this help\n\
  -V/--version                 Display tool version\n\
  -o/--output <filePath>       Set output directory path prefix\n\
  -v/--verbose                 Print extra detailed log information\n\
  -I/--path <filePath>         Add search path for imports\n\
  -g/--generate <language>     Select the output language (default is C)\n\
  -c/--codec <codecType>       Specify used codec type\n\
\n\
Available languages (use with -g option):\n\
  c    C/C++\n\
  py   Python\n\
\n\
Available codecs (use with --c option):\n\
  basic   BasicCodec\n\
\n";

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

/*!
 * @brief Class that encapsulates the erpcgen tool.
 *
 * A single global logger instance is created during object construction. It is
 * never freed because we need it up to the last possible minute, when an
 * exception could be thrown.
 */
class erpcgenTool
{
protected:
    enum verbose_type_t
    {
        kWarning,
        kInfo,
        kDebug,
        kExtraDebug
    }; /*!< Types of verbose outputs from erpcgen application. */

    enum languages_t
    {
        kCLanguage,
        kPythonLanguage,
    }; /*!< Generated outputs format. */

    typedef vector<string> string_vector_t; /*!< Vector of positional arguments. */

    int m_argc;                           /*!< Number of command line arguments. */
    char **m_argv;                        /*!< String value for each command line argument. */
    StdoutLogger *m_logger;               /*!< Singleton logger instance. */
    verbose_type_t m_verboseType;         /*!< Which type of log is need to set (warning, info, debug). */
    const char *m_outputFilePath;         /*!< Path to the output file. */
    const char *m_ErpcFile;               /*!< ERPC file. */
    string_vector_t m_positionalArgs;     /*!< Positional arguments. */
    languages_t m_outputLanguage;         /*!< Output language we're generating. */
    InterfaceDefinition::codec_t m_codec; /*!< Used codec type. */

public:
    /*!
     * @brief Constructor.
     *
     * @param[in] argc Count of arguments in argv variable.
     * @param[in] argv Pointer to array of arguments.
     *
     * Creates the singleton logger instance.
     */
    erpcgenTool(int argc, char *argv[])
    : m_argc(argc)
    , m_argv(argv)
    , m_logger(0)
    , m_verboseType(kWarning)
    , m_outputFilePath(NULL)
    , m_ErpcFile(NULL)
    , m_outputLanguage(kCLanguage)
    , m_codec(InterfaceDefinition::kNotSpecified)
    {
        // create logger instance
        m_logger = new StdoutLogger();
        m_logger->setFilterLevel(Logger::kWarning);
        Log::setLogger(m_logger);
    }

    /*!
     * @brief Destructor.
     */
    ~erpcgenTool() {}

    /*!
     * @brief Reads the command line options passed into the constructor.
     *
     * This method can return a return code to its caller, which will cause the
     * tool to exit immediately with that return code value. Normally, though, it
     * will return -1 to signal that the tool should continue to execute and
     * all options were processed successfully.
     *
     * The Options class is used to parse command line options. See
     * #k_optionsDefinition for the list of options and #k_usageText for the
     * descriptive help for each option.
     *
     * @retval -1 The options were processed successfully. Let the tool run normally.
     * @return A zero or positive result is a return code value that should be
     *      returned from the tool as it exits immediately.
     */
    int processOptions()
    {
        Options options(*m_argv, k_optionsDefinition);
        OptArgvIter iter(--m_argc, ++m_argv);

        // process command line options
        int optchar;
        const char *optarg;
        while ((optchar = options(iter, optarg)))
        {
            switch (optchar)
            {
                case '?':
                    printUsage(options);
                    return 0;

                case 'V':
                    printf("%s %s\n%s\n", k_toolName, k_version, k_copyright);
                    return 0;

                case 'o':
                    m_outputFilePath = optarg;
                    break;

                case 'v':
                    if (m_verboseType != kExtraDebug)
                    {
                        m_verboseType = (verbose_type_t)(((int)m_verboseType) + 1);
                    }
                    break;

                case 'I':
                    PathSearcher::getGlobalSearcher().addSearchPath(optarg);
                    break;

                case 'g': {
                    string lang = optarg;
                    if (lang == "c")
                    {
                        m_outputLanguage = kCLanguage;
                    }
                    else if (lang == "py")
                    {
                        m_outputLanguage = kPythonLanguage;
                    }
                    else
                    {
                        Log::error(format_string("error: unknown language %s", lang.c_str()).c_str());
                        return 1;
                    }
                    break;
                }

                case 'c': {
                    string codec = optarg;
                    if (codec.compare("basic") == 0)
                    {
                        m_codec = InterfaceDefinition::kBasicCodec;
                    }
                    else
                    {
                        Log::error(format_string("error: unknown codec type %s", codec.c_str()).c_str());
                        return 1;
                    }
                    break;
                }

                default:
                    Log::error("error: unrecognized option\n\n");
                    printUsage(options);
                    return 0;
            }
        }

        // handle positional args
        if (iter.index() < m_argc)
        {
            //            Log::debug("positional args:\n");
            int i;
            for (i = iter.index(); i < m_argc; ++i)
            {
                //                Log::debug("%d: %s\n", i - iter.index(), m_argv[i]);
                m_positionalArgs.push_back(m_argv[i]);
            }
        }

        // all is well
        return -1;
    }

    /*!
     * @brief Prints help for the tool.
     *
     * @param[in] options Options, which can be used.
     */
    void printUsage(Options &options)
    {
        options.usage(cout, "files...");
        printf(k_usageText);
    }

    /*!
     * @brief Core of the tool.
     *
     * Calls processOptions() to handle command line options before performing the
     * real work the tool does.
     *
     * @retval 1 The functions wasn't processed successfully.
     * @retval 0 The function was processed successfully.
     *
     * @exception Log::error This function is called, when function wasn't
     *              processed successfully.
     * @exception runtime_error Thrown, when positional args is empty.
     */
    int run()
    {
        try
        {
            // read command line options
            int result;
            if ((result = processOptions()) != -1)
            {
                return result;
            }

            // set verbose logging
            setVerboseLogging();

            // check argument values
            checkArguments();
            if (!m_positionalArgs.size())
            {
                throw runtime_error("no input file provided");
            }

            m_ErpcFile = m_positionalArgs[0].c_str();
            if (!m_outputFilePath)
            {
                m_outputFilePath = "";
            }

            // Parse and build definition model.
            InterfaceDefinition def;
            def.parse(m_ErpcFile);

            // Check for duplicate function IDs
            UniqueIdChecker uniqueIdCheck;
            uniqueIdCheck.makeIdsUnique(def);

            boost::filesystem::path filePath(m_ErpcFile);
            def.setProgramInfo(filePath.filename().generic_string(), m_outputFilePath, m_codec);

            switch (m_outputLanguage)
            {
                case kCLanguage:
                    CGenerator(&def).generate();
                    break;
                case kPythonLanguage:
                    PythonGenerator(&def).generate();
                    break;
            }
        }
        catch (exception &e)
        {
            Log::error("error: %s\n", e.what());
            return 1;
        }
        catch (...)
        {
            Log::error("error: unexpected exception\n");
            return 1;
        }

        return 0;
    }

    /*!
     * @brief Validate arguments that can be checked.
     *
     * @exception runtime_error Thrown if an argument value fails to pass validation.
     */
    void checkArguments()
    {
        //      if (m_outputFilePath == NULL)
        //      {
        //          throw runtime_error("no output file was specified");
        //      }
    }

    /*!
     * @brief Turns on verbose logging.
     */
    void setVerboseLogging()
    {
        // verbose only affects the INFO and DEBUG filter levels
        // if the user has selected quiet mode, it overrides verbose
        switch (m_verboseType)
        {
            case kWarning:
                Log::getLogger()->setFilterLevel(Logger::kWarning);
                break;
            case kInfo:
                Log::getLogger()->setFilterLevel(Logger::kInfo);
                break;
            case kDebug:
                Log::getLogger()->setFilterLevel(Logger::kDebug);
                break;
            case kExtraDebug:
                Log::getLogger()->setFilterLevel(Logger::kDebug2);
                break;
        }
    }
};

} // namespace erpcgen

/*!
 * @brief Main application entry point.
 *
 * Creates a tool instance and lets it take over.
 */
int main(int argc, char *argv[], char *envp[])
{
    (void)envp;
    try
    {
        return erpcgen::erpcgenTool(argc, argv).run();
    }
    catch (...)
    {
        Log::error("error: unexpected exception\n");
        return 1;
    }

    return 0;
}
