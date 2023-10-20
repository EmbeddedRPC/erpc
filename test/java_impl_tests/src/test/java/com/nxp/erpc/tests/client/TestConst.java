/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

package com.nxp.erpc.tests.client;

import com.nxp.erpc.tests.TestingClient;
import com.nxp.erpc.tests.test_arrays.test_unit_test_common.client.CommonClient;
import com.nxp.erpc.tests.test_const.erpc_outputs.test.common.Constants;
import com.nxp.erpc.tests.test_const.erpc_outputs.test.interfaces.IEmptyInterface;
import org.junit.jupiter.api.*;

import static org.junit.jupiter.api.Assertions.*;

@TestInstance(TestInstance.Lifecycle.PER_CLASS)
public class TestConst extends TestingClient {
    @Test
    public void runTestConstServiceID(){
        assertEquals(2, IEmptyInterface.SERVICE_ID);
    }

    @Test
    public void runTestCheckConsts(){
        assertEquals(3, Constants.a);
        assertEquals(3.14F, Constants.b);
        assertEquals("feedbabe", Constants.c);
        assertEquals(3.14F, Constants.d);
        assertEquals(11, Constants.x);
        assertEquals(20, Constants.y);
        assertEquals(100, Constants.mass);
        assertEquals(-9.8F, Constants.accel);
    }

    @AfterAll
    public void quit() {
        new CommonClient(clientManager).quit();
    }
}
