package com.nxp.erpc.tests.server;

import com.nxp.erpc.server.Service;
import com.nxp.erpc.tests.TestingServer;
import com.nxp.erpc.tests.server.services.CommonService;
import org.junit.jupiter.api.Test;

public class TestConstServer {

    @Test
    public void testConstServer(){
        TestingServer server = new TestingServer(new Service[]{ new CommonService()});
        server.run();
    }

}
