# Java implementation testing directory #

This directory contains acceptance tests for Java implementation.

Current implementation supports these tests:

- test_arrays
- test_binary
- test_builtin
- test_const
- test_enums
- test_lists
- test_struct

and communicate via TPC or serial port.

## Test folder structure

* **client/** - client side tests
* **server/** - server side tests
    * **servers/** - TCP and UART server for testing purpose
    * **services/** - server side functions implementation

## Generating shim code

To generate Java shim code for tests run:

```sh
ruby ..\..\internal\scripts\generate_erpc_java.rb [erpcgen_path] [erpc_path]
```

### Example

```sh
ruby ..\..\internal\scripts\generate_erpc_java.rb "erpcgen\VisualStudio_v14\Debug\" "C:\mcu-multicore\erpc"
```

## Running tests

To execute Java tests from commandline use Maven `test` lifecycle. Selecting specific test case is done with
parameter `-Dtest=TestName`. It is also possible to change transport configuration with parameters specified below.

`TestName` is derived as PascalCase from base test name specified in IDL file.

Example: `test_arrays -> TestArrays`

Server tests names are create by adding `Server` to `TestName`.

Example: `test_arrays -> TestArraysServer`

### Client

To run client test, execute `mvn test -Dtest=TestName`

### Server

To run server test, execute `mvn test -Dtest=TestNameServer -Dserver`

### Parameters

- `-Dtest=TEST_NAME` - Name of the test (Pascal case)
- `-Dclient` (**Default**) or `-Dserver`
- `-Dhost=HOST_NAME ` (default: localhost) - Set host name
- `-Dport=PORT` (default: 40) - Set port
- `-Dserial=COM_PORT` (default: COM6) - Enable serial communication
- `-Dbaudrate=BAUDRATE` (default: 115200) - Set serial link baudrate

```shell
mvn test -Dtest=TestArrays # Client test case
mvn test -Dtest=TestArrays -Dserial=COM6 -Dbaudrate=115200 # Specified serial transport
mvn test -Dtest=TestArraysServer -Dserver # Server test case
mvn test -Dtest=TestArraysServer -Dserver -Dport=12345 # Server test case with custom port
```

## Test folder structure

- **client/** - Client tests
- **server/** - Server tests
    - **services/** - Services for server tests
- **Connection.java** - Helper class for transport creation
- **TestingClient.java** - Base class for client tests
- **TestingServer.java**
  - This class represent ERPC server for testing purpose. SimpleServer instance is stored as static, so it
    is possible to stop the server and close connection with static function `stop()` without access to the instance of the
    class. This is used in `CommonService.java` to stop currently running server. 