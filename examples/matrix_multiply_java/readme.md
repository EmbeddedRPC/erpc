# Overview
This example demonstrates usage of Java eRPC as client. Server could be Python or C on Pc or boards (e.g. i.MX).

When client starts, it generates two random matrixes and sends them to the server. Server then performs matrix
multiplication and sends the result matrix back to the client. Client then prints the result matrix.
# eRPC documentation

eRPC is open-source project stored on github: https://github.com/EmbeddedRPC/erpc
eRPC documentation can be also found in: https://github.com/EmbeddedRPC/erpc/wiki

# Prerequisites

- Java 21 SDK
- Maven 3.9.4

# Example files

- `Main.java`: main example file
- `pom.xml`: Maven configuration file, with Java eRPC dependency
- `resource/erpc_matrix_multiply.erpc`: eRPC IDL file for example
- `erpc_matrix_multiply/`: eRPC output shim code generated from IDL file

# eRPC installation

1. Run `mvn -f ..\..\erpc_java\pom.xml install`. This command install Java eRPC to local repository.
2. Run `mvn install` to download all dependencies and install.

# Running the example

- Run `python ..\matrix_multiply_tcp_python\matrix_multiply.py --server --host localhost --port 40` as server.
- Run the `Main.java` as client.