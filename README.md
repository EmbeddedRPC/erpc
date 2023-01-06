# eRPC

[![Version](https://img.shields.io/github/v/release/EmbeddedRPC/erpc)](https://github.com/EmbeddedRPC/erpc/releases/latest)
[![Contributors](https://img.shields.io/github/contributors/EmbeddedRPC/erpc)](https://github.com/EmbeddedRPC/erpc/graphs/contributors)
[![Issues](https://img.shields.io/github/issues/EmbeddedRPC/erpc)](https://github.com/EmbeddedRPC/erpc/issues)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat-square)](https://github.com/EmbeddedRPC/erpc/pulls)
[![Gitpod ready-to-code](https://img.shields.io/badge/Gitpod-ready--to--code-blue?logo=gitpod)](https://gitpod.io/#https://github.com/EmbeddedRPC/erpc)

eRPC (Embedded RPC) is an open source Remote Procedure Call (RPC) system for multichip embedded systems and heterogeneous multicore SoCs.

Unlike other modern RPC systems, such as the excellent [Apache Thrift](http://thrift.apache.org), eRPC distinguishes itself by being designed for tightly coupled systems, using plain C for remote functions, and having a small code size (<5kB). It is not intended for high performance distributed systems over a network.

eRPC does not force upon you any particular API style. It allows you to export existing C functions, without having to change their prototypes. (There are limits, of course.) And although the internal infrastructure is written in C++, most users will be able to use only the simple C setup APIs shown in the examples below.

A code generator tool called `erpcgen` is included. It accepts input IDL files, having an `.erpc` extension, that have definitions of your data types and remote interfaces, and generates the shim code that handles serialization and invocation. `erpcgen` can generate either C/C++ or Python code.

Example `.erpc` file:

```java
// Define a data type.
enum LEDName { kRed, kGreen, kBlue }

// An interface is a logical grouping of functions.
interface IO {
    // Simple function declaration with an empty reply.
    set_led(LEDName whichLed, bool onOrOff) -> void
}
```

Client side usage:

```C
void example_client(void) {
    // Initialize client running over UART.
    erpc_client_init(
        erpc_transport_cmsis_uart_init(Driver_USART0),
        erpc_mbf_dynamic_init());

    // Now we can call the remote function to turn on the green LED.
    set_led(kGreen, true);
}
```

Server side usage:

```C
// Implement the remote function.
void set_led(LEDName whichLed, bool onOrOff) {
    // implementation goes here
}

void example_server(void) {
    // Initialize server running over UART.
    erpc_server_init(
        erpc_transport_cmsis_uart_init(Driver_USART0),
        erpc_mbf_dynamic_init());

    // Add the IO service.
    erpc_add_service_to_server(create_IO_service());

    // Run the server.
    erpc_server_run();
}
```

A number of transports are supported, and new transport classes are easy to write.

Supported transports:

* CMSIS UART
* NXP Kinetis SPI and DSPI
* POSIX and Windows serial port
* TCP/IP (mostly for testing)
* [NXP RPMsg-Lite / RPMsg TTY](https://github.com/NXPmicro/rpmsg-lite)
* SPIdev Linux
* USB CDC
* NXP Messaging Unit

eRPC is available with an unrestrictive BSD 3-clause license. See the [LICENSE file](https://github.com/EmbeddedRPC/erpc/blob/develop/LICENSE) for the full license text.

## Releases

[eRPC releases](https://github.com/EmbeddedRPC/erpc/releases)

### edge releases

Edge releases can by found on [eRPC CircleCI](https://app.circleci.com/pipelines/github/EmbeddedRPC/erpc) webpage. Choose build of interest, then platform target and choose ARTIFACTS tab. Here you can find binary application from chosen build.

## Documentation

[Documentation](https://github.com/EmbeddedRPC/erpc/wiki) is in the `wiki` section.

[eRPC Infrastructure documentation](https://embeddedrpc.github.io/)

## Examples

[Example IDL](examples/README.md) is available in the `examples/` folder.

Plenty of eRPC multicore and multiprocessor examples can be also found in NXP MCUXpressoSDK packages. Visit [https://mcuxpresso.nxp.com](https://mcuxpresso.nxp.com) to configure, build and download these packages.

To get the board list with multicore support (eRPC included) use filtering based on Middleware and search for 'multicore' string. Once the selected package with the multicore middleware is downloaded, see

<MCUXpressoSDK_install_dir>/boards/<board_name>/multicore_examples for eRPC multicore examples (RPMsg_Lite or Messaging Unit transports used) or

<MCUXpressoSDK_install_dir>/boards/<board_name>/multiprocessor_examples for eRPC multiprocessor examples (UART or SPI transports used).

eRPC examples use the 'erpc_' name prefix.

Another way of getting NXP MCUXpressoSDK eRPC multicore and multiprocessor examples is using the [mcux-sdk](https://github.com/NXPmicro/mcux-sdk) Github repo. Follow the description how to use the West tool
to clone and update the mcuxsdk repo in [readme Overview section](https://github.com/NXPmicro/mcux-sdk#overview). Once done the armgcc eRPC examples can be found in

mcuxsdk/examples/<board_name>/multicore_examples or in

mcuxsdk/examples/<board_name>/multiprocessor_examples folders.

You can use the evkmimxrt1170 as the board_name for instance. Similar to MCUXpressoSDK packages the eRPC examples use the 'erpc_' name prefix.

## References

This section provides links to interesting erpc-based projects, articles, blogs or guides:

* [erpc (EmbeddedRPC) getting started notes](https://programmersought.com/article/37585084512/)
* [ERPC Linux Local Environment Construction and Use](https://programmersought.com/article/88827920353/)
* [The New Wio Terminal eRPC Firmware](https://www.hackster.io/Salmanfarisvp/the-new-wio-terminal-erpc-firmware-bfd8bd)

## Directories

`doc` - Documentation.

`doxygen` - Configuration and support files for running Doxygen over the eRPC C++ infrastructure and erpcgen code.

`erpc_c` - Holds C/C++ infrastructure for eRPC. This is the code you will include in your application.

`erpc_python` - Holds Python version of the eRPC infrastructure.

`erpcgen` - Holds source code for erpcgen and makefiles or project files to build erpcgen on Windows, Linux, and OS X.

`erpcsniffer` - Holds source code for erpcsniffer application.

`examples` - Several example IDL files.

`mk` - Contains common makefiles for building eRPC components.

`test` - Client/server tests. These tests verify the entire communications path from client to server and back.

`utilities` - Holds utilities which bring additional benefit to eRPC apps developers.

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
* libboost-dev, libboost-filesystem-dev, libboost-system-dev: Boost C++ libraries (Linux needs to use libboost version 1.65.0)
* make: the GNU version of the 'make' utility
* python: Python language interpreter 3.6+ work
* gcc-7: GNU C compiler (recommended version)
* g++-7: GNU C++ compiler (recommended version)

Mandatory for case, when build for different architecture is needed

* gcc-multilib, g++-multilib
* boost libraries: for target architecture like libboost-filesystem-dev:i386 libboost-system-dev:i386

#### Mac OS X

Install these packages with [homebrew](http://brew.sh/):

* bison: GNU yacc-compatible parser generator (version 3.7.3 is recommended)
* flex: A fast lexical analyzer generator (version 2.6.4 is recommended)
* boost: Boost C++ libraries (version 1.74 is recommended)

### Building

To build the library and erpcgen, run from the repo root directory:

```sh
make
```

To install the library, erpcgen, and include files, run:

```sh
make install
```

You may need to sudo the `make install`.

By default this will install into `/usr/local`. If you want to install elsewhere, set the `PREFIX` environment variable. Example for installing into `/opt`:

```sh
make install PREFIX=/opt
```

List of top level Makefile targets:

* `erpc`: build the liberpc.a static library
* `erpcgen`: build the erpcgen tool
* `test`: build the unit tests under the `test/` directory
* `all`: build all of the above
* `install`: install liberpc.a, erpcgen, and include files

eRPC code is validated with respect to the C++ 11 standard.

### Installing for Python

To install the Python infrastructure for eRPC see instructions in the [erpc_python folder readme](erpc_python/readme.md).

## Known issues and limitations

* Static allocations controlled by the ERPC_ALLOCATION_POLICY config macro are not fully supported yet, i.e. not all erpc objects can be allocated statically now. It deals with the ongoing process and the full static allocations support will be added in the future.

## Code providing

Repository on Github contains two main branches. __Master__ and __develop__. Code is developed on __develop__ branch. Release version is created via merging __develop__ branch into __master__ branch.

---
Copyright 2014-2016 Freescale Semiconductor, Inc.

Copyright 2016-2021 NXP
