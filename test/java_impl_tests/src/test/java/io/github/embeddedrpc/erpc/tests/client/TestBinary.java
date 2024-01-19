/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

package io.github.embeddedrpc.erpc.tests.client;

import io.github.embeddedrpc.erpc.auxiliary.Reference;
import io.github.embeddedrpc.erpc.tests.TestingClient;
import io.github.embeddedrpc.erpc.tests.test_arrays.test_unit_test_common.client.CommonClient;
import io.github.embeddedrpc.erpc.tests.test_binary.erpc_outputs.test.client.BinaryClient;
import io.github.embeddedrpc.erpc.tests.test_binary.erpc_outputs.test.interfaces.IBinary;
import org.junit.jupiter.api.*;

import static org.junit.jupiter.api.Assertions.*;

@TestInstance(TestInstance.Lifecycle.PER_CLASS)
public class TestBinary extends TestingClient {
    private final BinaryClient client = new BinaryClient(clientManager);

    @Test
    public void runTestsendBinary() {
        client.sendBinary(new byte[]{0, 1, 2, 3, 4});
    }

    @Test
    public void runTestBinaryServiceID() {
        assertEquals(2, IBinary.SERVICE_ID);
    }

    @Test
    public void runTesttest_binary_allDirection() {
        byte[] a_binary = new byte[5];
        byte[] b_binary = new byte[5];
        Reference<byte[]> e_binary = new Reference<>(new byte[5]);

        for (byte i = 0; i < 5; i++) {
            a_binary[i] = i;
            b_binary[i] = (byte) (i * 2);
            e_binary.get()[i] = i;
        }

        client.test_binary_allDirection(a_binary, b_binary, e_binary);

        for (byte i = 0; i < 5; i++) {
            assertEquals(e_binary.get()[i], (a_binary[i] * b_binary[i]));
        }
    }

    @AfterAll
    public void quit() {
        new CommonClient(clientManager).quit();
    }
}
