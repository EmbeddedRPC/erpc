package com.nxp.erpc.tests.server;

import com.nxp.erpc.server.Service;
import com.nxp.erpc.tests.TestingServer;
import com.nxp.erpc.tests.server.services.CommonService;
import com.nxp.erpc.tests.server.services.TestEnumsService;
import org.junit.jupiter.api.Test;

public class TestEnumsServer {

    @Test
    public void testEnumServer() {
        TestingServer server = new TestingServer(new Service[]{new TestEnumsService(), new CommonService()});
        server.run();
    }
}
