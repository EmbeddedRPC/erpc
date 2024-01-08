package io.github.embeddedrpc.erpc.tests.server;

import io.github.embeddedrpc.erpc.server.Service;
import io.github.embeddedrpc.erpc.tests.TestingServer;
import io.github.embeddedrpc.erpc.tests.server.services.CommonService;
import io.github.embeddedrpc.erpc.tests.server.services.TestEnumsService;
import org.junit.jupiter.api.Test;

public class TestEnumsServer {

    @Test
    public void testEnumServer() {
        TestingServer server = new TestingServer(new Service[]{new TestEnumsService(), new CommonService()});
        server.run();
    }
}
