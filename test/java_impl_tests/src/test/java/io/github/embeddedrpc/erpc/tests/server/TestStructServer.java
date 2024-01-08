package io.github.embeddedrpc.erpc.tests.server;

import io.github.embeddedrpc.erpc.server.Service;
import io.github.embeddedrpc.erpc.tests.TestingServer;
import io.github.embeddedrpc.erpc.tests.server.services.CommonService;
import io.github.embeddedrpc.erpc.tests.server.services.TestStructService1;
import io.github.embeddedrpc.erpc.tests.server.services.TestStructService2;
import org.junit.jupiter.api.Test;

public class TestStructServer {
    @Test
    public void testListsServer() {
        TestingServer server = new TestingServer(new Service[]{
                new TestStructService1(),
                new TestStructService2(),
                new CommonService()
        });
        server.run();
    }
}
