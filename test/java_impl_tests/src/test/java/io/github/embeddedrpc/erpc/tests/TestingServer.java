package io.github.embeddedrpc.erpc.tests;

import io.github.embeddedrpc.erpc.codec.BasicCodecFactory;
import io.github.embeddedrpc.erpc.server.Server;
import io.github.embeddedrpc.erpc.server.Service;
import io.github.embeddedrpc.erpc.server.SimpleServer;
import io.github.embeddedrpc.erpc.transport.Transport;


public class TestingServer {
    private static Transport transport;
    private static Server server;

    public TestingServer(Service[] services) {
        transport = Connection.getConnectionFromSystemProperties();
        server = new SimpleServer(transport, new BasicCodecFactory());

        for (Service service : services) {
            server.addService(service);
        }
    }

    public void run() {
        if (server != null) {
            server.run();
        }
    }

    public static void stop() {
        if (server != null) {
            server.stop();
        }

        if (transport != null) {
            transport.close();
        }
    }
}
