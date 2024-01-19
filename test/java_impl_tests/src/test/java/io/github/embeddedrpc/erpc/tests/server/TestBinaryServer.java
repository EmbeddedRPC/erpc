package io.github.embeddedrpc.erpc.tests.server;

import io.github.embeddedrpc.erpc.server.Service;
import io.github.embeddedrpc.erpc.tests.TestingServer;
import io.github.embeddedrpc.erpc.tests.server.services.CommonService;
import io.github.embeddedrpc.erpc.tests.server.services.TestBinaryService;
import org.junit.jupiter.api.Test;

public class TestBinaryServer {

    @Test
    public void testBinaryServer() {
        TestingServer server = new TestingServer(new Service[]{new TestBinaryService(), new CommonService()});
        server.run();
    }
}
