# Matrix Multiply Java Example

This example demonstrates the use of Java eRPC. It could be used with as a server or client. This example is
compatible with the Python and C versions of this example.

When the client is started, it generates two random matrices and sends them to the server. The server then performs
matrix multiplication and sends the resulting matrix back to the client. The client then prints out the resulting
matrix.

## eRPC documentation

- eRPC is open-source project stored on github: https://github.com/EmbeddedRPC/erpc
- eRPC documentation can be also found on github: https://github.com/EmbeddedRPC/erpc/wiki

## Prerequisites

- [Java 21 SDK](https://jdk.java.net/21/)
- [Maven 3.9.5](https://maven.apache.org/download.cgi) 

To check that all the prerequisites are correctly installed, run `mvn -v`. You should get something like this:

```
Apache Maven 3.9.5 (***)
Maven home: c:\Program Files\maven\apache-maven-3.9.5
Java version: 21, vendor: Oracle Corporation, runtime: c:\Program Files\Java\jdk-21
Default locale: en_US, platform encoding: UTF-8
OS name: "***", version: "***", arch: "***", family: "***"
```

- If `mvn -v` fails, check that you have added maven to the `PATH` variable.
- If you do not see the correct Java version, check that you have set the `JAVA_HOME` and `PATH` variables correctly.

## Example's files

- `Main.java`: Main file
- `MatrixMultiplyService.java`: Custom service for MatrixMultiplyService interface
- `pom.xml`: Maven configuration file, with Java eRPC dependency
- `resource/erpc_matrix_multiply.erpc`: eRPC IDL file for example
- `erpc_matrix_multiply/`: eRPC output shim code generated from IDL file

## eRPC installation

1. Run `mvn -f ..\..\erpc_java\pom.xml install`. This command install the Java eRPC to the local repository.

## Building the example

To build and run this example, we create executable jar with copied dependencies using Maven.

1. Run `mvn package` to download all dependencies and create executable jar in `target/` (all Maven configuration is
   stored in `pom.xml`).

## Running the example

### Parameters

- `-c,--client      `  Run client
- `-s,--server      `  Run server
- `-S,--serial <arg>`  Serial device (default value is None)
- `-B,--baud <arg>  `  Baud (default value is 115200)
- `-t,--host <arg>  `  Host IP address (default value is localhost)
- `-p,--port <arg>  `  Port (default value is 40)

### Running

To execute the example use `java` with parameter `-jar` and the jar file we created. The pass the arguments to example.

```shell 
java -jar .\target\matrix_multiply_java-1.0.jar -s # Server on port 40
java -jar .\target\matrix_multiply_java-1.0.jar -s -p 12345 # Server on port 12345
java -jar .\target\matrix_multiply_java-1.0.jar -c # Client connected to `localhost:40`
java -jar .\target\matrix_multiply_java-1.0.jar -c -S COM6 # Client connected serial port COM6
```

You can also try this example with a Python or C implementation. For Python server, execute:
```shell
python ..\matrix_multiply_tcp_python\matrix_multiply.py --server --host localhost --port 40
```
and then:
```shell
java -jar .\target\matrix_multiply_java-1.0.jar -c
```