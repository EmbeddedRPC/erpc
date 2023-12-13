# eRPC

![GitHub all releases](https://img.shields.io/github/downloads/EmbeddedRPC/erpc/total)
[![GitHub License](https://img.shields.io/github/license/EmbeddedRPC/erpc)](/LICENSE)
[![Version](https://img.shields.io/github/v/release/EmbeddedRPC/erpc)](https://github.com/EmbeddedRPC/erpc/releases/latest)
![GitHub language count](https://img.shields.io/github/languages/count/EmbeddedRPC/erpc)
![GitHub top language](https://img.shields.io/github/languages/top/EmbeddedRPC/erpc)
![GitHub repo size](https://img.shields.io/github/repo-size/EmbeddedRPC/erpc)
![GitHub code size in bytes](https://img.shields.io/github/languages/code-size/EmbeddedRPC/erpc)
[![GitHub forks](https://img.shields.io/github/forks/EmbeddedRPC/erpc)](https://github.com/EmbeddedRPC/erpc/forks)
![GitHub Repo stars](https://img.shields.io/github/stars/EmbeddedRPC/erpc)
![GitHub watchers](https://img.shields.io/github/watchers/EmbeddedRPC/erpc)
![GitHub commit activity (branch)](https://img.shields.io/github/commit-activity/t/EmbeddedRPC/erpc)
![GitHub commits difference between two branches/tags/commits](https://img.shields.io/github/commits-difference/EmbeddedRPC/erpc?base=main&head=develop&label=master%20behind%20develop%20commits)
[![Contributors](https://img.shields.io/github/contributors/EmbeddedRPC/erpc)](https://github.com/EmbeddedRPC/erpc/graphs/contributors)
[![Issues](https://img.shields.io/github/issues/EmbeddedRPC/erpc)](https://github.com/EmbeddedRPC/erpc/issues)
[![PRs Welcome](https://img.shields.io/github/issues-pr/EmbeddedRPC/erpc)](https://github.com/EmbeddedRPC/erpc/pulls)

* [eRPC](#erpc)
  * [About](#about)
  * [Releases](#releases)
    * [Edge releases](#edge-releases)
  * [Documentation](#documentation)
  * [Examples](#examples)
  * [References](#references)
  * [Directories](#directories)
  * [Building and installing](#building-and-installing)
    * [Requirements](#requirements)
      * [Windows](#windows)
      * [Linux](#linux)
      * [Mac OS X](#mac-os-x)
    * [Building](#building)
    * [Installing for Python](#installing-for-python)
  * [Known issues and limitations](#known-issues-and-limitations)
  * [Code providing](#code-providing)

## About

[![Open enhancement issues](https://img.shields.io/github/issues/EmbeddedRPC/erpc/enhancement?labelColor=blue&color=black)](https://github.com/EmbeddedRPC/erpc/issues?q=is%3Aissue+is%3Aopen+label%3Aenhancement)
[![Closed enhancement issues](https://img.shields.io/github/issues-closed/EmbeddedRPC/erpc/enhancement?labelColor=blue&color=black)](https://github.com/EmbeddedRPC/erpc/issues?q=is%3Aissue+is%3Aclosed+label%3Aenhancement)
[![Open enhancement PRs](https://img.shields.io/github/issues-pr/EmbeddedRPC/erpc/enhancement?labelColor=blue&color=black)](https://github.com/EmbeddedRPC/erpc/pulls?q=is%3Apr+is%3Aopen+label%3Aenhancement)
[![Closed enhancement PRs](https://img.shields.io/github/issues-pr-closed/EmbeddedRPC/erpc/enhancement?labelColor=blue&color=black)](https://github.com/EmbeddedRPC/erpc/pulls?q=is%3Apr+is%3Aclosed+label%3Aenhancement)

[![Open bug issues](https://img.shields.io/github/issues/EmbeddedRPC/erpc/bug?labelColor=darkred&color=black)](https://github.com/EmbeddedRPC/erpc/issues?q=is%3Aissue+is%3Aopen+label%3Abug)
[![Closed bug issues](https://img.shields.io/github/issues-closed/EmbeddedRPC/erpc/bug?labelColor=darkred&color=black)](https://github.com/EmbeddedRPC/erpc/issues?q=is%3Aissue+is%3Aclosed+label%3Abug)
[![Open bug PRs](https://img.shields.io/github/issues-pr/EmbeddedRPC/erpc/bug?labelColor=darkred&color=black)](https://github.com/EmbeddedRPC/erpc/pulls?q=is%3Apr+is%3Aopen+label%3Abug)
[![Closed bug PRs](https://img.shields.io/github/issues-pr-closed/EmbeddedRPC/erpc/bug?labelColor=darkred&color=black)](https://github.com/EmbeddedRPC/erpc/pulls?q=is%3Apr+is%3Aclosed+label%3Abug)

[![Open question issues](https://img.shields.io/github/issues/EmbeddedRPC/erpc/question?labelColor=darkviolet&color=black)](https://github.com/EmbeddedRPC/erpc/issues?q=is%3Aissue+is%3Aopen+label%3Aquestion)
[![Closed question issues](https://img.shields.io/github/issues-closed/EmbeddedRPC/erpc/question?labelColor=darkviolet&color=black)](https://github.com/EmbeddedRPC/erpc/issues?q=is%3Aissue+is%3Aclosed+label%3Aquestion)

[![Open help-wanted issues](https://img.shields.io/github/issues/EmbeddedRPC/erpc/help%20wanted?labelColor=darkgreen&color=black)](https://github.com/EmbeddedRPC/erpc/issues?q=is%3Aissue+is%3Aopen+label%3A%22help%20wanted%22)
[![Closed help-wanted issues](https://img.shields.io/github/issues-closed/EmbeddedRPC/erpc/help%20wanted?labelColor=darkgreen&color=black)](https://github.com/EmbeddedRPC/erpc/issues?q=is%3Aissue+is%3Aclosed+label%3A%22help%20wanted%22)

eRPC (Embedded RPC) is an open source Remote Procedure Call (RPC) system for multichip embedded systems and heterogeneous multicore SoCs.

Unlike other modern RPC systems, such as the excellent [Apache Thrift](http://thrift.apache.org), eRPC distinguishes itself by being designed for tightly coupled systems, using plain C for remote functions, and having a small code size (<5kB). It is not intended for high performance distributed systems over a network.

eRPC does not force upon you any particular API style. It allows you to export existing C functions, without having to change their prototypes. (There are limits, of course.) And although the internal infrastructure is written in C++, most users will be able to use only the simple C setup APIs shown in the examples below.

A code generator tool called `erpcgen` is included. It accepts input IDL files, having an `.erpc` extension, that have definitions of your data types and remote interfaces, and generates the shim code that handles serialization and invocation. `erpcgen` can generate either C/C++ or Python code.

Example `.erpc` file:

```C++
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
    erpc_transport_t transport;
    erpc_mbf_t message_buffer_factory;
    erpc_client_t client_manager;

    /* Init eRPC client infrastructure */
    transport = erpc_transport_cmsis_uart_init(Driver_USART0);
    message_buffer_factory = erpc_mbf_dynamic_init();
    client_manager = erpc_client_init(transport, message_buffer_factory);

    /* init eRPC client IO service */
    initIO_client(client_manager);

    // Now we can call the remote function to turn on the green LED.
    set_led(kGreen, true);

    /* deinit objects */
    deinitIO_client();
    erpc_client_deinit(client_manager);
    erpc_mbf_dynamic_deinit(message_buffer_factory);
    erpc_transport_tcp_deinit(transport);
}
```

```C++
void example_client(void) {
    erpc_transport_t transport;
    erpc_mbf_t message_buffer_factory;
    erpc_client_t client_manager;

    /* Init eRPC client infrastructure */
    transport = erpc_transport_cmsis_uart_init(Driver_USART0);
    message_buffer_factory = erpc_mbf_dynamic_init();
    client_manager = erpc_client_init(transport, message_buffer_factory);

    /* scope for client service */
    {
        /* init eRPC client IO service */
        IO_client client(client_manager);

        // Now we can call the remote function to turn on the green LED.
        client.set_led(kGreen, true);
    }

    /* deinit objects */
    erpc_client_deinit(client_manager);
    erpc_mbf_dynamic_deinit(message_buffer_factory);
    erpc_transport_tcp_deinit(transport);
}
```

Server side usage:

```C
// Implement the remote function.
void set_led(LEDName whichLed, bool onOrOff) {
    // implementation goes here
}

void example_server(void) {
    erpc_transport_t transport;
    erpc_mbf_t message_buffer_factory;
    erpc_server_t server;
    erpc_service_t service = create_IO_service();

    /* Init eRPC server infrastructure */
    transport = erpc_transport_cmsis_uart_init(Driver_USART0);
    message_buffer_factory = erpc_mbf_dynamic_init();
    server = erpc_server_init(transport, message_buffer_factory);

    /* add custom service implementation to the server */
    erpc_add_service_to_server(server, service);

    // Run the server.
    erpc_server_run();

    /* deinit objects */
    destroy_IO_service(service);
    erpc_server_deinit(server);
    erpc_mbf_dynamic_deinit(message_buffer_factory);
    erpc_transport_tcp_deinit(transport);
}
```

```C++
// Implement the remote function.
class IO : public IO_interface
{
    /* eRPC call definition */
    void set_led(LEDName whichLed, bool onOrOff) override {
        // implementation goes here
    }
}

void example_server(void) {
    erpc_transport_t transport;
    erpc_mbf_t message_buffer_factory;
    erpc_server_t server;
    IO IOImpl;
    IO_service io(&IOImpl);

    /* Init eRPC server infrastructure */
    transport = erpc_transport_cmsis_uart_init(Driver_USART0);
    message_buffer_factory = erpc_mbf_dynamic_init();
    server = erpc_server_init(transport, message_buffer_factory);

    /* add custom service implementation to the server */
    erpc_add_service_to_server(server, &io);

    /* poll for requests */
    erpc_status_t err = server.run();

    /* deinit objects */
    erpc_server_deinit(server);
    erpc_mbf_dynamic_deinit(message_buffer_factory);
    erpc_transport_tcp_deinit(transport);
}
```

A number of transports are supported, and new transport classes are easy to write.

Supported transports can be found in [erpc/erpc_c/transport](/erpc_c/transports) folder. E.g:

* CMSIS UART
* NXP Kinetis SPI and DSPI
* POSIX and Windows serial port
* TCP/IP (mostly for testing)
* [NXP RPMsg-Lite / RPMsg TTY](https://github.com/nxp-mcuxpresso/rpmsg-lite)
* SPIdev Linux
* USB CDC
* NXP Messaging Unit

eRPC is available with an unrestrictive BSD 3-clause license. See the [LICENSE file](https://github.com/EmbeddedRPC/erpc/blob/develop/LICENSE) for the full license text.

## Releases

[eRPC releases](https://github.com/EmbeddedRPC/erpc/releases)

### Edge releases

Edge releases can by found on [eRPC CircleCI](https://app.circleci.com/pipelines/github/EmbeddedRPC/erpc) webpage. Choose build of interest, then platform target and choose ARTIFACTS tab. Here you can find binary application from chosen build.

## Documentation

[Documentation](https://github.com/EmbeddedRPC/erpc/wiki) is in the `wiki` section.

[eRPC Infrastructure documentation](https://embeddedrpc.github.io/)

## Examples

[Example IDL](examples/README.md) is available in the [examples/](/examples/) folder.

Plenty of eRPC multicore and multiprocessor examples can be also found in NXP MCUXpressoSDK packages. Visit [https://mcuxpresso.nxp.com](https://mcuxpresso.nxp.com) to configure, build and download these packages.

To get the board list with multicore support (eRPC included) use filtering based on Middleware and search for 'multicore' string. Once the selected package with the multicore middleware is downloaded, see

<MCUXpressoSDK_install_dir>/boards/<board_name>/multicore_examples for eRPC multicore examples (RPMsg_Lite or Messaging Unit transports used) or

<MCUXpressoSDK_install_dir>/boards/<board_name>/multiprocessor_examples for eRPC multiprocessor examples (UART or SPI transports used).

eRPC examples use the 'erpc_' name prefix.

Another way of getting NXP MCUXpressoSDK eRPC multicore and multiprocessor examples is using the [mcux-sdk](https://github.com/nxp-mcuxpresso/mcux-sdk) Github repo. Follow the description how to use the West tool
to clone and update the mcuxsdk repo in [readme Overview section](https://github.com/nxp-mcuxpresso/mcux-sdk#overview). Once done the armgcc eRPC examples can be found in

mcuxsdk/examples/<board_name>/multicore_examples or in

mcuxsdk/examples/<board_name>/multiprocessor_examples folders.

You can use the evkmimxrt1170 as the board_name for instance. Similar to MCUXpressoSDK packages the eRPC examples use the 'erpc_' name prefix.

## References

This section provides links to interesting erpc-based projects, articles, blogs or guides:

* [erpc (EmbeddedRPC) getting started notes](https://programmersought.com/article/37585084512/)
* [ERPC Linux Local Environment Construction and Use](https://programmersought.com/article/88827920353/)
* [The New Wio Terminal eRPC Firmware](https://www.hackster.io/Salmanfarisvp/the-new-wio-terminal-erpc-firmware-bfd8bd)

## Directories

[doc](/doc) - Documentation.

[doxygen](/doxygen) - Configuration and support files for running Doxygen over the eRPC C++ infrastructure and erpcgen code.

[erpc_c](/erpc_c) - Holds C/C++ infrastructure for eRPC. This is the code you will include in your application.

[erpc_python](/erpc_python) - Holds Python version of the eRPC infrastructure.

[erpcgen](/erpcgen) - Holds source code for erpcgen and makefiles or project files to build erpcgen on Windows, Linux, and OS X.

[erpcsniffer](/erpcsniffer) - Holds source code for erpcsniffer application.

[examples](/examples) - Several example IDL files.

[mk](/mk) - Contains common makefiles for building eRPC components.

[test](/test) - Client/server tests. These tests verify the entire communications path from client to server and back.

[utilities](/utilities) - Holds utilities which bring additional benefit to eRPC apps developers.

## Building and installing

These build instructions apply to host PCs and embedded Linux. For bare metal or RTOS embedded environments, you should copy the [erpc_c](/erpc_c) directory into your application sources.

The primary build system is makefile based. It builds a static library of the eRPC C/C++ infrastructure, the `erpcgen` executable, and optionally the unit tests.

The makefiles are compatible with gcc or clang on Linux, OS X, and Cygwin. A Windows build
of erpcgen using Visual Studio is also available in the [erpcgen/VisualStudio_v14](erpcgen/VisualStudio_v14) directory.
There is also an Xcode project file in the [erpcgen](/erpcgen) directory, which can be used to build erpcgen for OS X.

### Requirements

#### Windows

* Related to Visual Studio: steps are described in [`erpcgen/VisualStudio_v14/readme_erpcgen.txt`](erpcgen/VisualStudio_v14/readme_erpcgen.txt).
* mingw compilation can be used too

#### Linux

```bash
./install_dependencies.sh
```

Mandatory for case, when build for different architecture is needed

* gcc-multilib, g++-multilib

#### Mac OS X

```bash
./install_dependencies.sh
```

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
* `erpcsniffer`: build the sniffer tool
* `test`: build the unit tests under the [test](/test) directory
* `all`: build all of the above
* `install`: install liberpc.a, erpcgen, and include files

eRPC code is validated with respect to the C++ 11 standard.

### Installing for Python

To install the Python infrastructure for eRPC see instructions in the [erpc python readme](/erpc_python/readme.md).

## Known issues and limitations

* Static allocations controlled by the ERPC_ALLOCATION_POLICY config macro are not fully supported yet, i.e. not all erpc objects can be allocated statically now. It deals with the ongoing process and the full static allocations support will be added in the future.

## Code providing

Repository on Github contains two main branches: __main__ and __develop__. Code is developed on __develop__ branch. Release version is created via merging __develop__ branch into __main__ branch.

---
Copyright 2014-2016 Freescale Semiconductor, Inc.

Copyright 2016-2023 NXP
