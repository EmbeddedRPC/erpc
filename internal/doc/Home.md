# Before you begin
This reference helps software developers use Remote Procedure Calls (RPC) in embedded multicore microcontrollers (eRPC).

# What is the eRPC?
The eRPC (Embedded Remote Procedure Call) is a Remote Procedure Call (RPC) system created by NXP. An RPC is a mechanism used to invoke a software routine on a remote system using a simple local function call. The remote system may be any CPU connected by an arbitrary communications channel: a server across a network, another CPU core in a multicore system, and so on. To the client, it is just like calling a function in a library built into the application. The only difference is any latency or unreliability introduced by the communications channel.

When a remote function is called by the client:
* The function's parameters and an _identifier_ (for the called routine) are serialized into a stream of bytes.
* This byte stream is transported to the server through a communications channel (IPC, TPC/IP, UART, and so on).
* The server unserializes the parameters, determines which function was invoked, and calls it.
* If the function returns a value, then the value is serialized and sent back to the client over the communications channel.

[[images/RPCblockDiagram.png|alt=Remote procedure call system (RPC)]]

**_marshal_**: to arrange or prepare something in a clear, effective, or organized way. (from [Merriam-Webster dictionary](http://www.merriam-webster.com/) online) For RPC, to "marshal invocation and parameters" means to serialize the function call and its parameters for insertion into the byte stream communication channel. To "unmarshall invocation and parameters" means to unpack or deserialize the function call and its parameters.

The RPC system diagram only shows one direction of communication; it leaves out the reply from the server (to send the invoked function's return value to the client). This reply uses basically the same sequence of events, but flows back from the server to the client.

RPC implementations typically use a combination of a tool (eRPC generator from NXP) and an IDL (interface definition language) file to generate the source code to handle the details of collecting and serializing a function's parameters and building the data stream. The tool also generates code for a server-side shim that knows how to interpret a request and call the appropriate function. An IDL file is used to tell the generator tool about data types and RPC services.

## Why use NXP eRPC for multicore systems?
NXP eRPC offers:
* An easy way to create a client/server (core0/core1) application.
* Multiple platforms: NXP’s ``erpcgen`` tool is designed to be executable on most used operating systems (macOS®, Windows® OS, and Linux® OS).
* A simple changing transport layer (same applications can use a different transport medium).
* The server application can be _blocking_ (when a server serves only for client requests) or _non-blocking_ (when a server is also executing other code when the client requests).

eRPC high-level benefits:
* Lightweight but scalable
* Small generated code size
* Abstracted transport interface
* Abstracted serialization layer (it can be replaced)
* Small size of serialized data
* Designed to work well with C, but flexible enough to support object-oriented languages like C++
* Asynchronous notifications from server to client
* Multithreading of servers when built with an RTOS
* Unique specification of a function to be called
* Provisions for matching response messages to request messages
* Versioning of services
* Minimize any latency impact

## eRPC directory and subfolders
The eRPC files are organized using a top folder (directory) and subfolders underneath.

[[images/eRPC_directory.png|alt=RPC system]]

* ``erpc``: Contains all important files for the eRPC project layer, like primary ``Makefile``.
  * ``docs``: Contains eRPC documentation.
  * ``doxygen``: Contains doxygen configuration files for generating doxygen documentations.
  * ``erpc_c``: Contains source C/C++-code for the eRPC infrastructure.
     * ``config``: Contains the eRPC configuration files.
     * ``infra``: Contains C++ infrastructure code used to build server and client applications. For most use cases, the APIs in the setup/ folder are easier. Accessing the C++ layer directly is only required if you need to extend eRPC, or for atypical configurations.
     * ``port``: Contains the eRPC porting layer to adapt to different environments.
     * ``setup``: Contains a set of plain C APIs that wrap the C++ infrastructure, providing client and server init and deinit routines that greatly simplify eRPC usage in C-based projects. No knowledge of C++ is required to use these APIs.
     * ``transports``: Contains transport classes for the different methods of communication supported by eRPC. Some transports are applicable only to host PCs, while others are applicable only to embedded or multicore systems. Most transports have corresponding client and server setup functions, in the setup/ folder.
  * ``erpc_python``: Holds source PY-code for the eRPC infrastructure.
  * ``erpcgen``: Holds source code for erpcgen and makefiles, project files for Visual Studio
to build erpcgen on Windows OS, Linux OS, and OS X. This folder also contains the test folder for testing the generated code.
  * ``erpcsniffer``: Holds source code for erpcsniffer and makefiles to build erpcsniffer on Linux OS.
  * ``examples``: Contains IDL examples.
  * ``mk``: Contains makefiles, which contain the settings necessary for building the eRPC generator under different environments.
  * ``test``: Each ``test_xxx/`` subdirectory contains acceptance tests. These tests can be built on Linux OS or OS X with gcc using makefiles.
  * ``utilities``: Contains utilities which bring additional benefit to eRPC apps developers.

# How to create the IDL file
This section describes how to create an Interface Definition Language (IDL) file for the eRPC generator, and what the expected output looks like.

The eRPC generator tool uses an IDL file to describe the data types and service interfaces for which the generator tool generates the code. The IDL files for the eRPC generator normally have an ``.erpc`` extension.

## Output files
Typically, four files are generated for the C/C++ language output. When the file output name is not set (can be set in IDL file), the name of the IDL file is used.

Filename | Description
---|---
<_outputFileName_>.h | Common header file with types and interfaces
<_outputFileName_>_server.h | Server side header
<_outputFileName_>_client.cpp | Client shim implementation
<_outputFileName_>_server.cpp | Server shim implementation
erpc_crc16.h | Crc start value common for whole application

All type definitions are generated into the common ``<outputFileName>.h`` header file. This is the only file that clients need to include. The server-side projects need to include one more header (``<outputFileName>_server.h``), which contains the class definition for the server service class. The server initialization code must include this file (``<outputFileName>_server.h``), in order to create the service class instance.
