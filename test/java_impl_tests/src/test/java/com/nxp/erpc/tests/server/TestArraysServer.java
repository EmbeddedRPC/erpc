package com.nxp.erpc.tests.server;

import com.nxp.erpc.server.Service;
import com.nxp.erpc.tests.TestingServer;
import com.nxp.erpc.tests.server.services.CommonService;
import com.nxp.erpc.tests.server.services.TestArraysService;
import org.junit.jupiter.api.Test;

public class TestArraysServer {

    @Test
    public void testArraysServer() {
        TestingServer server = new TestingServer(new Service[]{new TestArraysService(), new CommonService()});
        server.run();
    }
}
