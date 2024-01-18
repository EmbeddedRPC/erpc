package io.github.embeddedrpc.erpc.tests.server;

import io.github.embeddedrpc.erpc.server.Service;
import io.github.embeddedrpc.erpc.tests.TestingServer;
import io.github.embeddedrpc.erpc.tests.server.services.CommonService;
import io.github.embeddedrpc.erpc.tests.server.services.TestBuiltinService;
import org.junit.jupiter.api.Test;

public class TestBuiltinServer {

    @Test
    public void testBuiltinServer() {
        TestingServer server = new TestingServer(new Service[]{new TestBuiltinService(), new CommonService()});
        server.run();
    }
}
