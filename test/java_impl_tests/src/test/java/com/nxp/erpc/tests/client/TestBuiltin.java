/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

package com.nxp.erpc.tests.client;

import com.nxp.erpc.auxiliary.Reference;
import com.nxp.erpc.tests.TestingClient;
import com.nxp.erpc.tests.test_arrays.test_unit_test_common.client.CommonClient;
import com.nxp.erpc.tests.test_builtin.erpc_outputs.test.client.BuiltinServicesClient;
import com.nxp.erpc.tests.test_builtin.erpc_outputs.test.interfaces.IBuiltinServices;
import org.junit.jupiter.api.*;

import static org.junit.jupiter.api.Assertions.*;

@TestInstance(TestInstance.Lifecycle.PER_CLASS)
@TestMethodOrder(MethodOrderer.MethodName.class)
class TestBuiltin extends TestingClient {
    private final int gInt32AClient = 2;
    private final int gInt32BClient = -20;
    private final String gStringAClient = "Hello";
    private final String gStringBClient = "World!";

    private final BuiltinServicesClient client = new BuiltinServicesClient(clientManager);

    @Test
    public void runTestEnumsServiceID() {
        assertEquals(2, IBuiltinServices.SERVICE_ID);
    }

    @Test
    public void runTesttest_int32_in_out() {
        Reference<Integer> int32C = new Reference<>(0);

        System.out.println("RPC request is sent to the server");
        client.test_int32_in(gInt32AClient);

        System.out.println("RPC request is sent to the server");
        client.test_int32_out(int32C);

        assertEquals(gInt32AClient, int32C.get());
    }

    @Test
    public void runTesttest_int32_inout() {
        Reference<Integer> int32E = new Reference<>(-6);

        for (int i = -5; i < 5; i++) {
            System.out.println("RPC request is sent to the server");
            client.test_int32_inout(int32E);
            assertEquals(i, int32E.get());
        }
    }

    @Test
    public void runTestest_int32_return() {
        int int32R = 0;

        System.out.println("RPC request is sent to the server");
        client.test_int32_in(gInt32AClient);

        System.out.println("RPC request is sent to the server");
        client.test_int32_in2(gInt32BClient);

        System.out.println("RPC request is sent to the server");
        int32R = client.test_int32_return();

        assertEquals((gInt32AClient * gInt32BClient), int32R);
    }

    @Test
    public void runTestest_int32_allDirection() {
        Reference<Integer> int32C = new Reference<>(0);
        Reference<Integer> int32E = new Reference<>(7);
        int int32R = 0;

        System.out.println("RPC request is sent to the server");
        int32R = client.test_int32_allDirection(gInt32AClient, gInt32BClient, int32C, int32E);

        assertEquals(int32C.get(), gInt32AClient);
        assertEquals(14, int32E.get());
        assertEquals(int32E.get(), 14);
        assertEquals((gInt32AClient * gInt32BClient), int32R);
    }
    @Test
    public void runTestest_float_inout() {
        float a = 3.14F;
        Reference<Float> b = new Reference<>(0.0F);

        System.out.println("RPC request is sent to the server");
        client.test_float_inout(a, b);

        assertEquals(b.get(), a);
    }

    @Test
    public void runTestest_double_inout() {
        double a = 3.14F;
        Reference<Double> b = new Reference<>(0.0D);

        System.out.println("RPC request is sent to the server");
        client.test_double_inout(a, b);

        assertEquals(b.get(), a);
    }

    @Test
    @Order(1)
    public void runTest1test_string_in_out() {
        System.out.println("1");
        Reference<String> stringC = new Reference<>("");

        System.out.println("RPC request is sent to the server");
        client.test_string_in(gStringAClient);
        System.out.println("RPC request is sent to the server");
        client.test_string_out(stringC);

        assertEquals(gStringAClient, stringC.get());
    }

    @Test
    @Order(2)
    public void runTest2test_string_inout() {
        System.out.println("2");
        Reference<String> stringE = new Reference<>(gStringAClient + " ");

        System.out.println("RPC request is sent to the server");
        client.test_string_in2(gStringBClient);
        System.out.println("RPC request is sent to the server");
        client.test_string_inout(stringE);

        assertEquals("Hello World!", stringE.get());
    }

    @Test
    @Order(3)
    public void runTest3test_string_return() {
        System.out.println("3");
        String stringR = "";

        System.out.println("RPC request is sent to the server");
        stringR = client.test_string_return();

        assertEquals("Hello World!", stringR);
    }

    @Test
    public void runTestest_string_allDirection() {
        Reference<String> stringC = new Reference<>("");
        Reference<String> stringE = new Reference<>(gStringAClient + " ");
        String stringR = "";

        System.out.println("RPC request is sent to the server");
        stringR = client.test_string_allDirection(gStringAClient, gStringBClient, stringC, stringE);

        assertEquals(stringC.get(), gStringAClient);
        assertEquals(stringE.get(), "Hello World!");
        assertEquals(stringR, "Hello World!");
    }

    @Test
    public void runTesStringParamTest1() {
        int int32R = -1;

        System.out.println("RPC request is sent to the server");
        int32R = client.sendHello("Hello World!");

        assertEquals(int32R, 0);
    }

    @Test
    public void runTesStringParamTest2() {
        int int32R = -1;

        System.out.println("RPC request is sent to the server");
        int32R = client.sendTwoStrings("String one.", "String two.");

        assertEquals(int32R, 0);
    }

    @Test
    public void runTesStringReturnTest1() {
        System.out.println("RPC request is sent to the server");
        assertEquals("Hello", client.returnHello());
    }

    @AfterAll
    public void quit() {
        new CommonClient(clientManager).quit();
    }
}