package com.nxp.erpc.tests.server;

import com.nxp.erpc.server.Service;
import com.nxp.erpc.tests.TestingServer;
import com.nxp.erpc.tests.server.services.CommonService;
import com.nxp.erpc.tests.server.services.TestBinaryService;
import org.junit.jupiter.api.Test;

public class TestBinaryServer {

    @Test
    public void testBinaryServer() {
        TestingServer server = new TestingServer(new Service[]{new TestBinaryService(), new CommonService()});
        server.run();
    }
}
