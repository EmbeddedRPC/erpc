package io.github.embeddedrpc.erpc.tests.server;

import io.github.embeddedrpc.erpc.server.Service;
import io.github.embeddedrpc.erpc.tests.TestingServer;
import io.github.embeddedrpc.erpc.tests.server.services.CommonService;
import io.github.embeddedrpc.erpc.tests.server.services.TestArraysService;
import org.junit.jupiter.api.Test;

public class TestArraysServer {

    @Test
    public void testArraysServer() {
        TestingServer server = new TestingServer(new Service[]{new TestArraysService(), new CommonService()});
        server.run();
    }
}
