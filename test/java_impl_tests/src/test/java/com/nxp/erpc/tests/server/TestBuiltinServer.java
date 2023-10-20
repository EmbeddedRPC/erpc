package com.nxp.erpc.tests.server;

import com.nxp.erpc.server.Service;
import com.nxp.erpc.tests.TestingServer;
import com.nxp.erpc.tests.server.services.CommonService;
import com.nxp.erpc.tests.server.services.TestBuiltinService;
import org.junit.jupiter.api.Test;

public class TestBuiltinServer {

    @Test
    public void testBuiltinServer() {
        TestingServer server = new TestingServer(new Service[]{new TestBuiltinService(), new CommonService()});
        server.run();
    }
}
