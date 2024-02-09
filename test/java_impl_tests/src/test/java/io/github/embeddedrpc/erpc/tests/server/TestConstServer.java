package io.github.embeddedrpc.erpc.tests.server;

import io.github.embeddedrpc.erpc.server.Service;
import io.github.embeddedrpc.erpc.tests.TestingServer;
import io.github.embeddedrpc.erpc.tests.server.services.CommonService;
import org.junit.jupiter.api.Test;

public class TestConstServer {

    @Test
    public void testConstServer(){
        TestingServer server = new TestingServer(new Service[]{ new CommonService()});
        server.run();
    }

}
