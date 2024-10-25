# eRPC Java Infrastructure

This folder contains the Java implementation of the eRPC infrastructure.

## Contents

- [/src/main/java/io/github/embeddedrpc/erpc](./src/main/java/io/github/embeddedrpc/erpc) - Java package containing eRPC infrastructure.
- [pom.xml](./pom.xml) - Maven config file.
- [checkstyle.xml](./checkstyle.xml) - Check style config file.

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


## Installation

1. Run `mvn install` to download all dependencies and install eRPC to local maven repository.

## Usage

### Client

```Java
import  io.github.embeddedrpc.erpc.auxiliary.Reference;
import  io.github.embeddedrpc.erpc.client.ClientManager;
import  io.github.embeddedrpc.erpc.codec.BasicCodecFactory;
import  io.github.embeddedrpc.erpc.transport.TCPTransport;
import  io.github.embeddedrpc.erpc.transport.Transport;

import com.example.app.client.MyTestClient;

public class Main {
    public static void main(String[] args) {
        Transport transport = new TCPTransport("localhost", 40);
        //Transport transport = new SerialTransport("COM4", 115200);

        ClientManager clientManager = new ClientManager(transport, new BasicCodecFactory());
        MyTestClient client = new MyTestClient(clientManager);

        Reference<String> out;
        int result;
        
        result = client.foo(42, 8);
        client.boo("John", out);
    }
}
```

### Server
#### Main.java
```Java
import  io.github.embeddedrpc.erpc.server.Server;
import  io.github.embeddedrpc.erpc.server.SimpleServer;
import  io.github.embeddedrpc.erpc.codec.BasicCodecFactory;
import  io.github.embeddedrpc.erpc.transport.TCPTransport;
import  io.github.embeddedrpc.erpc.transport.Transport;

import com.example.app.MyTestService;

public class Main {
    public static void main(String[] args) {
        Transport transport = new TCPTransport("localhost", 40);
        //Transport transport = new SerialTransport("COM4", 115200);

        Server server = new SimpleServer(transport, new BasicCodecFactory());

        server.addService(new MyTestService());

        server.run();
    }
}
```
#### MyTestService.java

```Java
import  io.github.embeddedrpc.erpc.auxiliary.Reference;
import com.example.app.server.AbstractMyTestService;

public class MyTestService extends AbstractMyTestService {
    @Override
    public int foo(int a, int b) {
        return a + b;
    }

    @Override
    public void boo(String in, Reference<String> out) {
        out.set("Hello " + in);
    }
}
```