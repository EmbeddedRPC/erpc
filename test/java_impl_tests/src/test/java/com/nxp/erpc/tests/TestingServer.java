package com.nxp.erpc.tests;

import com.nxp.erpc.codec.BasicCodecFactory;
import com.nxp.erpc.server.Server;
import com.nxp.erpc.server.Service;
import com.nxp.erpc.server.SimpleServer;
import com.nxp.erpc.transport.Transport;


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
