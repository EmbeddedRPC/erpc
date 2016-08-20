# eRPC

eRPC (Embedded RPC) is an open source Remote Procedure Call (RPC) system for multichip embedded systems and heterogeneous multicore SoCs.

Unlike other modern RPC systems, such as the excellent [Apache Thrift](http://thrift.apache.org), eRPC distinguishes itself by being designed for tightly coupled systems, using plain C for remote functions, and having a small code size (<5kB). It is not intended for high performance distributed systems over a network.

eRPC does not force upon you any particular API style. It allows you to export existing C functions, without having to change their prototypes. (There are limits, of course.) And although the internal infrastructure is written in C++, most users will be able to use only the simple C APIs shown in the examples below.

A code generator tool called `erpcgen` is included. It accepts input IDL files, having an `.erpc` extension, that have definitions of your data types and remote interfaces, and generates the shim code that handles serialization and invocation. `erpcgen` can generate either C/C++ or Python code.

Example `.erpc` file:

~~~~~{java}
// Define a data type.
enum LEDName { kRed, kGreen, kBlue }

// An interface is a logical grouping of functions.
interface IO {
    // Simple function declaration with an empty reply.
    set_led(LEDName whichLed, bool onOrOff) -> void
}
~~~~~

Client side usage:
~~~~~{c}
void example_client(void) {
    // Initialize client running over UART.
    erpc_client_init(
        erpc_transport_uart_init(UART0, 115200, get_uart_clock()));

    // Now we can call the remote function to turn on the green LED.
    set_led(kGreen, true);
}
~~~~~

Server side usage:
~~~~~{c}
// Implement the remote function.
void set_led(LEDName whichLed, bool onOrOff) {
    // implementation goes here
}

void example_server(void) {
    // Initialize server running over UART.
    erpc_server_init(
        erpc_transport_uart_init(UART0, 115200, get_uart_clock()));

    // Add the IO service.
    erpc_add_service_to_server(create_IO_service());

    // Run the server.
    erpc_server_run();
}
~~~~~

A number of transports are supported, and new transport classes are easy to write.

Supported transports:

* Kinetis UART and LPUART
* Kinetis SPI and DSPI
* POSIX and Windows serial port
* TCP/IP (mostly for testing)
* [OpenAMP RPMsg](https://github.com/OpenAMP/open-amp)
* NXP RPMsg-Lite


## Documentation

[Documentation](doc/README.md) is in the `doc/` folder.

[Example IDL](examples/README.md) is available in the `examples/` folder.


## Directories

`doc` - Documentation.

`doxygen` - Configuration and support files for running Doxygen over the eRPC C++ infrastructure and erpcgen code.

`erpc_c` - Holds C/C++ infrastructure for eRPC. This is the code you will include in your application.

`erpc_python` - Holds Python version of the eRPC infrastructure.

`erpcgen` - Holds source code for erpcgen and makefiles or project files to build erpcgen on Windows, Linux, and OS X.

`mk` - Contains common makefiles for building eRPC components.

`test` - Client/server tests. These tests verify the entire communications path from client to server and back.

## Build environment setup

### Windows

Steps are described in `erpcgen/VisualStudio_v12/readme_erpcgen.txt`.

### Linux

Install these packages:
* bison: GNU yacc-complatible parser generator
* flex: A fast lexical analyzer generator
* libboost-devel: Boost C++ libraries
* make: the GNU version of the 'make' utility
* python: Python language interpreter (either 2.7 or 3.5+ work)
* gcc-core: GNU Compiler Collection (C, OpenMP)
* gcc-g++: GNU Compiler Collection (C++)

### Mac OS X

Install these packages with homebrew:
* bison: GNU yacc-complatible parser generator
* flex: A fast lexical analyzer generator
* boost: Boost C++ libraries

### MinGW:

1. mingw32-make, ming32-gcc, mingw32-g++.
2. In Windows enviroment install Python 2.7 or 3.5+.
3. Unzip `erpcgen/VisualStudio_v12/win_flex_bison-latest.zip` into same directory (without subdirectory).
4. Download and build boost libraries into `C:\boost_1_57_0`.
5. Add this path to Windows PATH enviroment variable: `C:\MinGW\bin`.
6. Call mingw32-make instead of make from the command line.

## Building

The primary build system is makefile based. The main Makefile in the root directory can build
all targets. The default target is "erpcgen". Building the "all" target will build the unit tests under
the `test/` directory in addition to erpcgen.

These makefiles are compatible with gcc or clang on Linux, OS X, MinGW, and Cygwin. A Windows build
of erpcgen using Visual Studio is also available in the `erpcgen/VisualStudio_v12/` directory.
There is also an Xcode project file in the `erpcgen/` directory which can be used to build erpcgen
for OS X.

After the makefiles finish building, the erpcgen executable will be located in `Debug/<OS>/erpcgen/`
from the repo root directory.

