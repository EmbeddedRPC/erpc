# eRPC

eRPC (Embedded RPC) is an open source Remote Procedure Call (RPC) system for multichip embedded systems and heterogeneous multicore SoCs.

Unlike other modern RPC systems, such as the excellent [Apache Thrift](http://thrift.apache.org), eRPC distinguishes itself by being designed for tightly coupled systems, using plain C for remote functions, and having a small code size (<5kB). It is not intended for high performance distributed systems over a network.

eRPC does not force upon you any particular API style. It allows you to export existing C functions, without having to change their prototypes. (There are limits, of course.) And although the internal infrastructure is written in C++, most users will be able to use only the simple C setup APIs shown in the examples below.

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
        erpc_transport_cmsis_uart_init(Driver_USART0);

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
        erpc_transport_cmsis_uart_init(Driver_USART0);

    // Add the IO service.
    erpc_add_service_to_server(create_IO_service());

    // Run the server.
    erpc_server_run();
}
~~~~~

A number of transports are supported, and new transport classes are easy to write.

Supported transports:

* CMSIS UART
* NXP Kinetis SPI and DSPI
* POSIX and Windows serial port
* TCP/IP (mostly for testing)
* [NXP RPMsg-Lite](https://github.com/NXPmicro/rpmsg-lite)

eRPC is available with an unrestrictive BSD 3-clause license. See the LICENSE file for the full license text.

## Releases

[eRPC releases](https://github.com/EmbeddedRPC/erpc/releases)

## Documentation

[Documentation](https://github.com/EmbeddedRPC/erpc/wiki) is in the `wiki` section. Commit sha in wiki repository: 431cba8.

[Example IDL](examples/README.md) is available in the `examples/` folder.


## Directories

`doc` - Documentation.

`doxygen` - Configuration and support files for running Doxygen over the eRPC C++ infrastructure and erpcgen code.

`erpc_c` - Holds C/C++ infrastructure for eRPC. This is the code you will include in your application.

`erpc_python` - Holds Python version of the eRPC infrastructure.

`erpcgen` - Holds source code for erpcgen and makefiles or project files to build erpcgen on Windows, Linux, and OS X.

`examples` - Several example IDL files.

`mk` - Contains common makefiles for building eRPC components.

`test` - Client/server tests. These tests verify the entire communications path from client to server and back.


## Building and installing

These build instructions apply to host PCs and embedded Linux. For bare metal or RTOS embedded environments, you should copy the `erpc_c` directory into your application sources.

The primary build system is makefile based. It builds a static library of the eRPC C/C++ infrastructure, the `erpcgen` executable, and optionally the unit tests.

The makefiles are compatible with gcc or clang on Linux, OS X, and Cygwin. A Windows build
of erpcgen using Visual Studio is also available in the `erpcgen/VisualStudio_v14/` directory.
There is also an Xcode project file in the `erpcgen/` directory which can be used to build erpcgen
for OS X.

### Requirements

#### Windows

Steps are described in [`erpcgen/VisualStudio_v14/readme_erpcgen.txt`](erpcgen/VisualStudio_v14/readme_erpcgen.txt).

#### Linux and Cygwin

Install these packages:
* bison: GNU yacc-compatible parser generator
* flex: A fast lexical analyzer generator
* libboost-dev, libboost-filesystem-dev, libboost-system-dev: Boost C++ libraries (Linux needs to use libboost version 1.58.0)
* make: the GNU version of the 'make' utility
* python: Python language interpreter (either 2.7 or 3.5+ work)
* gcc-core: GNU Compiler Collection (C, OpenMP)
* gcc-g++: GNU Compiler Collection (C++)

Mandatory for case, when build for different architecture is needed
* gcc-multilib, g++-multilib
* boost libraries: for target architecture like libboost-filesystem-dev:i386 libboost-system-dev:i386

#### Mac OS X

Install these packages with [homebrew](http://brew.sh/):
* bison: GNU yacc-compatible parser generator
* flex: A fast lexical analyzer generator
* boost: Boost C++ libraries

### Building

To build the library and erpcgen, run from the repo root directory:

    % make

To install the library, erpcgen, and include files, run:

    % make install

You may need to sudo the `make install`.

By default this will install into `/usr/local`. If you want to install elsewhere, set the `PREFIX` environment variable. Example for installing into `/opt`:

    % make install PREFIX=/opt

List of top level Makefile targets:
- `erpc`: build the liberpc.a static library
- `erpcgen`: build the erpcgen tool
- `test`: build the unit tests under the `test/` directory
- `all`: build all of the above
- `install`: install liberpc.a, erpcgen, and include files

### Installing for Python

To install the Python infrastructure for eRPC, first change to the `erpc_python/` directory. Then run the setup.py script like this:

    python setup.py install

After installation, the `erpc` package is available via normal import statements. See the [erpc_python folder readme](erpc_python/readme.md) for more.

## Code providing:

Repository on Github contains two main branches. __Master__ and __develop__. Code is developed on __develop__ branch. Release version is created via merging __develop__ branch into __master__ branch.

---
Copyright 2014-2016 Freescale Semiconductor, Inc.

Copyright 2016-2020 NXP
