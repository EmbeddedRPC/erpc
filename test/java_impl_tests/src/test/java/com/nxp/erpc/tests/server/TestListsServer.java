package com.nxp.erpc.tests.server;

import com.nxp.erpc.server.Service;
import com.nxp.erpc.tests.TestingServer;
import com.nxp.erpc.tests.server.services.CommonService;
import com.nxp.erpc.tests.server.services.TestListsService;
import org.junit.jupiter.api.Test;

public class TestListsServer {

    @Test
    public void testListsServer() {
        TestingServer server = new TestingServer(new Service[]{new TestListsService(), new CommonService()});
        server.run();
    }
}