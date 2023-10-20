# eRPC Java Infrastructure

This folder contains the Java implementation of the eRPC infrastructure.

## Contents

`/src/main/java/com/nxp/erpc` - Java package for eRPC infrastructure.

`pom.xml` - Maven config file.

`checkstyle.xml` - Check style config file.

## Prerequisites

- Java 21 SDK
- Maven 3.9.4

## Installation

1. Run `mvn install` to download all dependencies and install eRPC to local maven repository.

## Usage

### Client

```Java
import com.nxp.erpc.auxiliary.Reference;
import com.nxp.erpc.client.ClientManager;
import com.nxp.erpc.codec.BasicCodecFactory;
import com.nxp.erpc.transport.TCPTransport;
import com.nxp.erpc.transport.Transport;

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
import com.nxp.erpc.server.Server;
import com.nxp.erpc.server.SimpleServer;
import com.nxp.erpc.codec.BasicCodecFactory;
import com.nxp.erpc.transport.TCPTransport;
import com.nxp.erpc.transport.Transport;

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
import com.nxp.erpc.auxiliary.Reference;
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