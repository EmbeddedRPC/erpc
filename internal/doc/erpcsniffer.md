# Create and use the eRPC sniffer tool
This chapter describes how to get and use the eRPC sniffer tool (_erpcsniffer_), a tool that catches and shows parsed eRPC messages. The supported OS is Linux.

## Pre-generated eRPC generator tool

The tool can be found on [eRPC releases](https://github.com/EmbeddedRPC/erpc/releases) section on github page.

## How to create an eRPC sniffer tool
* The _erpcsniffer_ specific source files are located in the ``erpc/erpcsniffer/src`` directory. The tool is using source files also from ``erpc/erpcgen/src`` and ``erpc/erpc_c`` directories.
* To build the _erpcsniffer_ application on Linux OS, the compiler (gcc, g++), make, flex/bison applications, and boost libraries are needed.
* On the Linux OS, build the application using the command (which must be executed in the ``erpc/`` directory):

```
make erpcsniffer
```

The output _erpcsniffer_ application is located in the ``erpc/Debug/<os_type>/erpcsniffer/`` folder. The ``<os_type>`` is set automatically, based on which build system is used.

## How to use an eRPC sniffer tool
When the _erpcsniffer_ application is available, you can run it from the command line using the appropriate command line options.

### _erpcsniffer_ command line options
###### ``-?`` or ``—help``
Shows supported commands.

###### ``-V`` or ``—version``
Displays tool version.

###### ``-o <filePath>`` or ``—output<filePath>``
Sets output file path.

###### ``-v`` or ``—verbose``
* When the verbose option is **not used**, it prints only warnings and errors.
* When the verbose option is **used once**, it prints standard output information.
* When the verbose option is **used twice or more**, it prints debugging information.

###### ``-I <filePath>`` or ``—path <filePath>``
Add search path for imports.

###### ``-t <transport>`` or ``—transport<transport>``
Sets transport type (tcp, serial, ...) to use for catching erpc messages.

###### ``-q <quantity>`` or ``—quantity <quantity>``
How much messages to catch. 0 - infinity.

###### ``-b <baudrate>`` or ``—baudrate <baudrate>``
Sets baud rate for serial transport.

###### ``-p <port>`` or ``—port<port>``
Sets port name for serial transport or port number for tcp transport.

###### ``-h <host>`` or ``—host <host>``
Sets host name for tcp transport.

Example of how to use the eRPC sniffer is:
```
erpcsniffer -t tcp -p <port> -h <hostname> -o <outputfilname> <idlFileName>
```
_For example:_
```
erpcsniffer -t tcp -p 54321 -h localhost -o erpcOutput.txt MyFirstErpcApp.erpc
```

### Steps
1. One of multicore applications needs be built with enabled _ERPC_MESSAGE_LOGGING_ macro and the project needs include message loggers files stored in ``erpc/erpc_c`` folder. This application also needs to initialize another eRPC transport, which communicates with the eRPCsniffer tool. This transport needs to be set to the client/server object through add message logger function.
2. Start the eRPCsniffer tool with the correct input parameters.
3. Start the multicore application.

### Example

This is an example demonstrating the calling function with the array inout parameter. This array parameter contains numbers from 0 to 11.

IDL:
```
interface DemoService
{
    sendReceivedInt32(inout int32[12] arrayNumbers) -> void
}
```

Received messages (output from erpcsniffer):
```
1. 'Request' message (id: 0) at 08:49:15 07/26/17. Sequence number 1. Time from last message 0 ns.
Group name:
Interface name:DemoService id:1
Function name:sendReceivedInt32 id:1 prototype: void sendReceivedInt32(int32[12] arrayNumbers);
  Param 'arrayNumbers':
    [0]: int32_t value: 0
    [1]: int32_t value: 1
    [2]: int32_t value: 2
    [3]: int32_t value: 3
    [4]: int32_t value: 4
    [5]: int32_t value: 5
    [6]: int32_t value: 6
    [7]: int32_t value: 7
    [8]: int32_t value: 8
    [9]: int32_t value: 9
    [10]:int32_t value: 10
    [11]:int32_t value: 11

2. 'Answer' message (id: 2) at 08:49:15 07/26/17. Sequence number 1. Time from last message 1 296 301 ns.
Group name:
Interface name:PointersService id:1
Function name:sendReceivedInt32 id:1 prototype: void sendReceivedInt32(int32[12] arrayNumbers);
  Param 'arrayNumbers':
    [0]: int32_t value: 0
    [1]: int32_t value: 1
    [2]: int32_t value: 2
    [3]: int32_t value: 3
    [4]: int32_t value: 4
    [5]: int32_t value: 5
    [6]: int32_t value: 6
    [7]: int32_t value: 7
    [8]: int32_t value: 8
    [9]: int32_t value: 9
    [10]:int32_t value: 10
    [11]:int32_t value: 11
```
