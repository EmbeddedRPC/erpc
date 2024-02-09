package io.github.embeddedrpc.erpc.tests.server;

import io.github.embeddedrpc.erpc.server.Service;
import io.github.embeddedrpc.erpc.tests.TestingServer;
import io.github.embeddedrpc.erpc.tests.server.services.CommonService;
import io.github.embeddedrpc.erpc.tests.server.services.TestListsService;
import org.junit.jupiter.api.Test;

public class TestListsServer {

    @Test
    public void testListsServer() {
        TestingServer server = new TestingServer(new Service[]{new TestListsService(), new CommonService()});
        server.run();
    }
}