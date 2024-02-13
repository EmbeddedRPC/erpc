package org.example;

import io.github.embeddedrpc.erpc.client.ClientManager;
import io.github.embeddedrpc.erpc.codec.BasicCodecFactory;
import io.github.embeddedrpc.erpc.transport.TCPTransport;
import io.github.embeddedrpc.erpc.transport.Transport;
import org.example.hello_world.client.TextServiceClient;

import java.io.IOException;

public class Client {
    public void run() throws IOException {
        Config cfg = new Config("../../config.h");

        Transport transport = new TCPTransport(cfg.getString("ERPC_HOSTNAME"), cfg.getInt("ERPC_PORT"));
        ClientManager clientManager = new ClientManager(transport, new BasicCodecFactory());
        TextServiceClient client = new TextServiceClient(clientManager);

        client.printText("Hello world!");
    }
}
