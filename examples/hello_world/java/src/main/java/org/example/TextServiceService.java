package org.example;

import io.github.embeddedrpc.erpc.server.SimpleServer;
import org.example.hello_world.server.AbstractTextServiceService;

public class TextServiceService extends AbstractTextServiceService {
    SimpleServer server;

    public TextServiceService(SimpleServer g_server) {
        this.server = g_server;
    }

    @Override
    public boolean printText(String text) {
        System.out.println(text);
        return true;
    }

    @Override
    public void stopServer() {
        server.stop();
    }
}
