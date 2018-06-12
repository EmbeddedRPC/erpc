/*
 * The Clear BSD License
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 * that the following conditions are met:
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

#include "erpc_crc16.h"
#include "erpc_tcp_transport.h"
#include "erpc_transport.h"
#include "erpc_transport_setup.h"
#include "erpc_version.h"
#include "ErpcLexer.h"
#include "InterfaceDefinition.h"
#include "Logging.h"
#include "SearchPath.h"
#include "Sniffer.h"
#include "UniqueIdChecker.h"
#include "annotations.h"
#include "options.h"
#include <cstdint>
#include <cstdlib>
#include <stdexcept>
#include <vector>

/*!
 * @brief Entry point for the tool.
 */
int main(int argc, char *argv[], char *envp[]);

using namespace erpc;
using namespace erpcgen;
using namespace std;
namespace erpcsniffer {

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

/*! The tool's name. */
const char k_toolName[] = "erpcsniffer";

/*! Current version number for the tool. */
const char k_version[] = ERPC_VERSION;

/*! Copyright string. */
const char k_copyright[] = "Copyright 2017 NXP. All rights reserved.";

static const char *k_optionsDefinition[] = { "?|help",
                                             "V|version",
                                             "o:output <filePath>",
                                             "v|verbose",
                                             "I:path <filePath>",
                                             "t:transport <transport>",
                                             "q:quantity <quantity>",
                                             "b:baudrate <baudrate>",
                                             "p:port <port>",
                                             "h:host <host>",
                                             NULL };

/*! Help string. */
const char k_usageText[] =
    "\nOptions:\n\
  -?/--help                    Show this help\n\
  -V/--version                 Display tool version\n\
  -o/--output <filePath>       Set path to output file (file name included)\n\
  -v/--verbose                 Print extra detailed log information\n\
  -I/--path <filePath>         Add search path for imports\n\
  -t/--transport <transport>   Type of transport.\n\
  -q/--quantity <quantity>     Record messages count (0 - infinity).\n\
  -b/--baudrate <baudrate>     Baud rate.\n\
  -p/--port <port>             Port name or port number.\n\
  -h/--host <host>             Host definition.\n\
\n\
Available transports (use with -t option):\n\
  tcp      Tcp transport type (host, port number).\n\
  serial   Serial transport type (port name, baud rate).\n\
\n";

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

/*!
 * @brief Class that encapsulates the erpcsniffer tool.
 *
 * A single global logger instance is created during object construction. It is
 * never freed because we need it up to the last possible minute, when an
 * exception could be thrown.
 */
class erpcsnifferTool
{
protected:
    enum verbose_type_t
    {
        kWarning,
        kInfo,
        kDebug,
        kExtraDebug
    }; /*!< Types of verbose outputs from erpcsniffer application. */

    enum transports_t
    {
        kNoneTransport,
        kTcpTransport,
        kSerialTransport
    }; /*!< Type of transport to use. */

    typedef vector<string> string_vector_t;

    int m_argc;                   /*!< Number of command line arguments. */
    char **m_argv;                /*!< String value for each command line argument. */
    StdoutLogger *m_logger;       /*!< Singleton logger instance. */
    verbose_type_t m_verboseType; /*!< Which type of log is need to set (warning, info, debug). */
    const char *m_outputFilePath; /*!< Path to the output file. */
    const char *m_ErpcFile;       /*!< ERPC file. */
    string_vector_t m_positionalArgs;
    transports_t m_transport; /*!< Transport used for receiving messages. */
    uint64_t m_quantity;      /*!< Quantity of logs to store. */
    uint32_t m_baudrate;      /*!< Baudrate rate speed. */
    const char *m_port;       /*!< Name or number of port. Based on used transport. */
    const char *m_host;       /*!< Host name */

public:
    /*!
     * @brief Constructor.
     *
     * @param[in] argc Count of arguments in argv variable.
     * @param[in] argv Pointer to array of arguments.
     *
     * Creates the singleton logger instance.
     */
    erpcsnifferTool(int argc, char *argv[])
    : m_argc(argc)
    , m_argv(argv)
    , m_logger(0)
    , m_verboseType(kWarning)
    , m_outputFilePath(NULL)
    , m_ErpcFile(NULL)
    , m_transport(kNoneTransport)
    , m_quantity(10)
    , m_baudrate(115200)
    , m_port(NULL)
    , m_host(NULL)
    {
        // create logger instance
        m_logger = new StdoutLogger();
        m_logger->setFilterLevel(Logger::kWarning);
        Log::setLogger(m_logger);
    }

    /*!
     * @brief Destructor.
     */
    ~erpcsnifferTool() {}

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
                {
                    printUsage(options);
                    return 0;
                }

                case 'V':
                {
                    printf("%s %s\n%s\n", k_toolName, k_version, k_copyright);
                    return 0;
                }

                case 'o':
                {
                    m_outputFilePath = optarg;
                    break;
                }

                case 'v':
                {
                    if (m_verboseType != kExtraDebug)
                    {
                        m_verboseType = (verbose_type_t)(((int)m_verboseType) + 1);
                    }
                    break;
                }

                case 'I':
                {
                    PathSearcher::getGlobalSearcher().addSearchPath(optarg);
                    break;
                }

                case 't':
                {
                    string transport = optarg;
                    if (transport == "tcp")
                    {
                        m_transport = kTcpTransport;
                    }
                    else if (transport == "serial")
                    {
                        m_transport = kSerialTransport;
                    }
                    else
                    {
                        Log::error(format_string("error: unknown transport type %s", transport.c_str()).c_str());
                        return 1;
                    }
                    break;
                }

                case 'q':
                {
                    m_quantity = strtoul(optarg, NULL, 10);
                    break;
                }

                case 'b':
                {
                    m_baudrate = strtoul(optarg, NULL, 10);
                    break;
                }

                case 'p':
                {
                    m_port = optarg;
                    break;
                }

                case 'h':
                {
                    m_host = optarg;
                    break;
                }

                default:
                {
                    Log::error("error: unrecognized option\n\n");
                    printUsage(options);
                    return 0;
                }
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

            // Check for duplicate function IDs
            UniqueIdChecker uniqueIdCheck;
            uniqueIdCheck.makeIdsUnique(def);

            Transport *_transport;
            switch (m_transport)
            {
                case kTcpTransport:
                {
                    uint16_t portNumber = strtoul(m_port, NULL, 10);
                    TCPTransport *tcpTransport = new TCPTransport(m_host, portNumber, true);
                    if (erpc_status_t err = tcpTransport->open())
                    {
                        return err;
                    }
                    _transport = tcpTransport;
                    break;
                }

                case kSerialTransport:
                {
                    erpc_transport_t transport = erpc_transport_serial_init(m_port, m_baudrate);
                    _transport = reinterpret_cast<Transport *>(transport);
                    assert(_transport);
                    break;
                }

                default:
                {
                    break;
                }
            }

            Crc16 crc;
            if (def.hasProgramSymbol())
            {
                Program *program = def.getProgramSymbol();
                if (program->findAnnotation(CRC_ANNOTATION, Annotation::kC) != nullptr)
                {
                    crc.setCrcStart(def.getIdlCrc16());
                }
            }
            _transport->setCrc16(&crc);
            Sniffer s(_transport, &def, m_outputFilePath, m_quantity);
            return s.run();
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

} // namespace erpcsniffer

/*!
 * @brief Main application entry point.
 *
 * Creates a tool instance and lets it take over.
 */
int main(int argc, char *argv[], char *envp[])
{
    try
    {
        return erpcsniffer::erpcsnifferTool(argc, argv).run();
    }
    catch (...)
    {
        Log::error("error: unexpected exception\n");
        return 1;
    }

    return 0;
}
