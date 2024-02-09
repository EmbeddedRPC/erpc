/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

package io.github.embeddedrpc.erpc.tests.client;

import io.github.embeddedrpc.erpc.auxiliary.Reference;
import io.github.embeddedrpc.erpc.tests.TestingClient;
import io.github.embeddedrpc.erpc.tests.test_arrays.test_unit_test_common.client.CommonClient;
import io.github.embeddedrpc.erpc.tests.test_enums.erpc_outputs.test.client.EnumsServiceClient;
import io.github.embeddedrpc.erpc.tests.test_enums.erpc_outputs.test.common.enums.enumColor;
import io.github.embeddedrpc.erpc.tests.test_enums.erpc_outputs.test.common.enums.enumColor2;
import io.github.embeddedrpc.erpc.tests.test_enums.erpc_outputs.test.common.enums.enumErrorCode;
import io.github.embeddedrpc.erpc.tests.test_enums.erpc_outputs.test.interfaces.IEnumsService;
import org.junit.jupiter.api.*;

import static io.github.embeddedrpc.erpc.tests.test_enums.erpc_outputs.test.common.enums.enumErrorCode.ERROR_NONE;
import static io.github.embeddedrpc.erpc.tests.test_enums.erpc_outputs.test.common.enums.enumErrorCode.ERROR_UNKNOWN;
import static org.junit.jupiter.api.Assertions.*;

@TestInstance(TestInstance.Lifecycle.PER_CLASS)
public class TestEnums extends TestingClient {
    private final enumColor gEnumColorAClient = enumColor.green;
    private final enumColor gEnumColorBClient = enumColor.red;

    private final EnumsServiceClient client = new EnumsServiceClient(clientManager);

    @Test
    public void runTestEnumsServiceID() {
        assertEquals(2, IEnumsService.SERVICE_ID);
    }

    @Test
    public void runTesttest_enumColor_in_out() {
        Reference<enumColor> enumColorC = new Reference<>();

        System.out.println("RPC request is sent to the server");
        client.test_enumColor_in(gEnumColorAClient);

        System.out.println("RPC request is sent to the server");
        client.test_enumColor_out(enumColorC);

        assertEquals(enumColorC.get(), gEnumColorAClient);
    }

    @Test
    public void runTesttest_enumColor_inout() {
        Reference<enumColor> enumColorE = new Reference<>(gEnumColorAClient);

        client.test_enumColor_inout(enumColorE);

        assertEquals(enumColorE.get(), gEnumColorBClient);
    }

    @Test
    public void runTesttest_enumColor_return() {
//        client.test_enumColor_in(gEnumColorBClient);
//        client.test_enumColor_in(gEnumColorBClient);

    }

    @Test
    public void runTesttest_enumColor_allDirection() {
        Reference<enumColor> enumColorC = new Reference<>();
        Reference<enumColor> enumColorE = new Reference<>(gEnumColorAClient);
        enumColor enumColorR;

        enumColorR = client.test_enumColor_allDirection(gEnumColorAClient, gEnumColorBClient, enumColorC, enumColorE);

        assertEquals(gEnumColorAClient, enumColorC.get());
        assertEquals(gEnumColorBClient, enumColorE.get());
        assertEquals(gEnumColorAClient, enumColorR);
    }

    @Test
    public void runTesttest_enumColor2_allDirection() {
        enumColor2 enumColor2A = enumColor2.pink;
        enumColor2 enumColor2B = enumColor2.yellow;
        Reference<enumColor2> enumColor2C = new Reference<>();
        Reference<enumColor2> enumColor2E = new Reference<>(enumColor2A);
        enumColor2 enumColor2R;

        enumColor2R = client.test_enumColor2_allDirection(enumColor2A, enumColor2B, enumColor2C, enumColor2E);

        assertEquals(enumColor2A, enumColor2C.get());
        assertEquals(enumColor2B, enumColor2E.get());
        assertEquals(enumColor2A, enumColor2R);
    }

    @Test
    public void runTesttest_enumErrorCode_allDirection() {
        enumErrorCode enumErrorCodeA = ERROR_NONE;
        enumErrorCode enumErrorCodeB = ERROR_UNKNOWN;
        Reference<enumErrorCode> enumErrorCodeC = new Reference<>();
        Reference<enumErrorCode> enumErrorCodeE = new Reference<>(ERROR_NONE);
        enumErrorCode enumErrorCodeR;

        enumErrorCodeR = client.test_enumErrorCode_allDirection(enumErrorCodeA, enumErrorCodeB, enumErrorCodeC, enumErrorCodeE);

        assertEquals(enumErrorCodeA, enumErrorCodeC.get());
        assertEquals(enumErrorCodeB, enumErrorCodeE.get());
        assertEquals(enumErrorCodeA, enumErrorCodeR);
    }

    @AfterAll
    public void quit() {
        new CommonClient(clientManager).quit();
    }
}
