package io.github.embeddedrpc.erpc.tests.server.services;

import io.github.embeddedrpc.erpc.tests.TestingServer;
import io.github.embeddedrpc.erpc.tests.test_const.erpc_outputs.test_unit_test_common.server.AbstractCommonService;

public class CommonService extends AbstractCommonService {

    @Override
    public void quit() {
        TestingServer.stop();
    }

    @Override
    public int getServerAllocated() {
        return 0;
    }
}
