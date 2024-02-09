package io.github.embeddedrpc.erpc.tests.server;

import io.github.embeddedrpc.erpc.server.Service;
import io.github.embeddedrpc.erpc.tests.TestingServer;
import io.github.embeddedrpc.erpc.tests.server.services.CommonService;
import io.github.embeddedrpc.erpc.tests.server.services.TestAnnotationsService;
import org.junit.jupiter.api.Test;

public class TestAnnotationsServer {
    @Test
    public void testAnnotationsServer() {
        TestingServer server = new TestingServer(new Service[]{new TestAnnotationsService(), new CommonService()});
        server.run();
    }
}
