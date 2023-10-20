package com.nxp.erpc.tests.server;

import com.nxp.erpc.server.Service;
import com.nxp.erpc.tests.TestingServer;
import com.nxp.erpc.tests.server.services.CommonService;
import com.nxp.erpc.tests.server.services.TestTypedefService;
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
