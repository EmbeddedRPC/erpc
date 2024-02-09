package io.github.embeddedrpc.erpc.tests.server;

import io.github.embeddedrpc.erpc.server.Service;
import io.github.embeddedrpc.erpc.tests.TestingServer;
import io.github.embeddedrpc.erpc.tests.server.services.CommonService;
import io.github.embeddedrpc.erpc.tests.server.services.TestTypedefService;
import org.junit.jupiter.api.Test;

public class TestTypedefServer {
    @Test
    public void testTypedefServer() {
        TestingServer server = new TestingServer(new Service[]{
                new TestTypedefService(),
                new CommonService()
        });
        server.run();
    }
}
