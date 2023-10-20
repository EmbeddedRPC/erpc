package com.nxp.erpc.tests.server;

import com.nxp.erpc.server.Service;
import com.nxp.erpc.tests.TestingServer;
import com.nxp.erpc.tests.server.services.CommonService;
import com.nxp.erpc.tests.server.services.TestAnnotationsService;
import org.junit.jupiter.api.Test;

public class TestAnnotationsServer {
    @Test
    public void testAnnotationsServer() {
        TestingServer server = new TestingServer(new Service[]{new TestAnnotationsService(), new CommonService()});
        server.run();
    }
}
