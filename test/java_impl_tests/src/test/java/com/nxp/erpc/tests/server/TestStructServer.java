package com.nxp.erpc.tests.server;

import com.nxp.erpc.server.Service;
import com.nxp.erpc.tests.TestingServer;
import com.nxp.erpc.tests.server.services.CommonService;
import com.nxp.erpc.tests.server.services.TestStructService1;
import com.nxp.erpc.tests.server.services.TestStructService2;
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
