package org.example;

import io.github.embeddedrpc.erpc.codec.BasicCodecFactory;
import io.github.embeddedrpc.erpc.server.SimpleServer;
import io.github.embeddedrpc.erpc.transport.TCPTransport;
import io.github.embeddedrpc.erpc.transport.Transport;

import java.io.IOException;

public class Server {
    public void run() throws IOException {
        Config cfg = new Config("../../config.h");

        Transport transport = new TCPTransport(cfg.getInt("ERPC_PORT"));
        io.github.embeddedrpc.erpc.server.Server server = new SimpleServer(transport, new BasicCodecFactory());

        server.addService(new TextServiceService());

        server.run();
    }
}
