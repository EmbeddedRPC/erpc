/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

package io.github.embeddedrpc.erpc.tests.client;

import io.github.embeddedrpc.erpc.auxiliary.Reference;
import io.github.embeddedrpc.erpc.tests.TestingClient;
import io.github.embeddedrpc.erpc.tests.test_arrays.test_unit_test_common.client.CommonClient;
import io.github.embeddedrpc.erpc.tests.test_lists.erpc_outputs.test.client.PointersServiceClient;
import io.github.embeddedrpc.erpc.tests.test_lists.erpc_outputs.test.common.enums.enumColor;
import io.github.embeddedrpc.erpc.tests.test_lists.erpc_outputs.test.common.structs.C;
import io.github.embeddedrpc.erpc.tests.test_lists.erpc_outputs.test.common.structs.gapAdStructure_t;
import io.github.embeddedrpc.erpc.tests.test_lists.erpc_outputs.test.common.structs.gapAdvertisingData_t;
import io.github.embeddedrpc.erpc.tests.test_lists.erpc_outputs.test.common.structs.listStruct;
import io.github.embeddedrpc.erpc.tests.test_lists.erpc_outputs.test.interfaces.IPointersService;
import org.junit.jupiter.api.*;

import java.util.ArrayList;
import java.util.List;

import static org.junit.jupiter.api.Assertions.*;

@TestInstance(TestInstance.Lifecycle.PER_CLASS)
public class TestLists extends TestingClient {
    private final PointersServiceClient client = new PointersServiceClient(clientManager);

    @Test
    public void runTestListsServiceID() {
        assertEquals(2, IPointersService.SERVICE_ID);
    }

    @Test
    public void runSendReceivedInt32() {
        List<Integer> sendList = new ArrayList<>(12);
        List<Integer> receivedList;

        for (int i = 0; i < 12; i++) {
            sendList.add(i);
        }

        receivedList = client.sendReceivedInt32(sendList);

        for (int i = 0; i < receivedList.size(); i++) {
            assertEquals(sendList.get(i), receivedList.get(i) / 2);
        }
    }

    @Test
    public void runSendReceiveZeroSize() {
        List<Integer> sendList = List.of(0);
        List<Integer> receivedList;

        receivedList = client.sendReceivedInt32(sendList);

        assertEquals(List.of(0), receivedList);
    }

    @Test
    public void runSendReceived2Int32() {
        List<List<Integer>> sendList = new ArrayList<>();
        List<List<Integer>> receivedList;

        for (int i = 0; i < 10; i++) {
            List<Integer> x = new ArrayList<>();
            for (int j = 0; j < i + 1; j++) {
                x.add(j);
            }
            sendList.add(x);
        }

        receivedList = client.sendReceived2Int32(sendList);

        for (int x = 0; x < receivedList.size(); x++) {
            for (int y = 0; y < receivedList.get(x).size(); y++) {
                assertEquals(sendList.get(x).get(y), receivedList.get(x).get(y) / 2);
            }
        }
    }

    @Test
    public void runSendReceivedEnum() {
        List<enumColor> sendList = List.of(enumColor.red, enumColor.green, enumColor.blue);
        List<enumColor> receivedList;

        receivedList = client.sendReceivedEnum(sendList);

        assertEquals(sendList, receivedList);
    }

    @Test
    public void runSendReceived2Enum() {
        List<List<enumColor>> sendList = new ArrayList<>();
        List<List<enumColor>> receivedList;

        for (int a = 0; a < 3; a++) {
            List<enumColor> x = new ArrayList<>();
            for (int b = 0; b < a + 1; b++) {
                switch (b) {
                    case 0 -> x.add(enumColor.red);
                    case 1 -> x.add(enumColor.green);
                    case 2 -> x.add(enumColor.blue);
                }
            }
            sendList.add(x);
        }

        receivedList = client.sendReceived2Enum(sendList);

        assertEquals(sendList, receivedList);
    }

    @Test
    public void runSendReceivedStruct() {
        List<C> sendList = new ArrayList<>();
        List<C> receivedList;

        for (int a = 0; a < 10; a++) {
            sendList.add(new C(a, a * 2));
        }

        receivedList = client.sendReceivedStruct(sendList);

        for (int x = 0; x < receivedList.size(); x++) {
            assertEquals(sendList.get(x).get_m(), receivedList.get(x).get_m() / 2);
            assertEquals(sendList.get(x).get_n(), receivedList.get(x).get_n() / 2);
        }
    }

    @Test
    public void runSendReceived2Struct() {
        List<List<C>> sendList = new ArrayList<>();
        List<List<C>> receivedList;

        for (int a = 0; a < 3; a++) {
            List<C> x = new ArrayList<>();
            for (int b = 0; b < a + 1; b++) {
                x.add(new C(a, b));
            }
            sendList.add(x);
        }

        receivedList = client.sendReceived2Struct(sendList);

        for (int x = 0; x < receivedList.size(); x++) {
            for (int y = 0; y < receivedList.get(x).size(); y++) {
                assertEquals(sendList.get(x).get(y).get_m(), receivedList.get(x).get(y).get_m() / 2);
                assertEquals(sendList.get(x).get(y).get_n(), receivedList.get(x).get(y).get_n() / 2);
            }
        }
    }

    @Test
    public void runSendReceivedString() {
        List<String> sendList = new ArrayList<>();
        List<String> receivedList;

        for (int i = 0; i < 12; i++) {
            sendList.add("Hello");
        }

        receivedList = client.sendReceivedString(sendList);

        for (String x : receivedList) {
            assertEquals("Hello World!", x);
        }
    }

    @Test
    public void runSendReceived2String() {
        List<List<String>> sendList = new ArrayList<>();
        List<List<String>> receivedList;

        for (int a = 0; a < 12; a++) {
            List<String> x = new ArrayList<>();
            for (int b = 0; b < a + 1; b++) {
                x.add("Hello");
            }
            sendList.add(x);
        }

        receivedList = client.sendReceived2String(sendList);

        for (List<String> strings : receivedList) {
            for (String string : strings) {
                assertEquals("Hello World!", string);
            }
        }
    }

    @Test
    public void runtest_list_allDirection() {
        List<Long> sendListA = new ArrayList<>();
        List<Long> sendListB = new ArrayList<>();
        Reference<List<Long>> sendListC = new Reference<>(new ArrayList<>());

        for (long i = 0; i < 10; i++) {
            sendListA.add(i);
            sendListB.add(i * 2);
            sendListC.get().add(6 * i);
        }

        client.test_list_allDirection(sendListA, sendListB, sendListC);

        for (int x = 0; x < sendListA.size(); x++) {
            assertEquals(12 * sendListA.get(x), sendListC.get().get(x));
        }
    }

    @Test
    public void runtestLengthAnnotation() {
        long length = 5;
        List<Integer> list = new ArrayList<>();
        int result;

        for (int i = 0; i < length; i++) {
            list.add(i + 1);
        }

        result = client.testLengthAnnotation(list);

        assertEquals(1, result);
    }

    @Test
    public void runtestLengthAnnotationInStruct() {
        listStruct myListStruct = new listStruct(new ArrayList<>());
        int result;

        for (int x = 0; x < 5; x++) {
            myListStruct.get_myList().add(x + 1);
        }

        result = client.testLengthAnnotationInStruct(myListStruct);

        assertEquals(10, result);
    }

    @Test
    public void runreturnSentStructLengthAnnotation() {
        listStruct myListStruct = new listStruct(new ArrayList<>());
        listStruct returnStruct;

        for (int x = 0; x < 5; x++) {
            myListStruct.get_myList().add(x + 1);
        }

        returnStruct = client.returnSentStructLengthAnnotation(myListStruct);

        for (int x = 0; x < returnStruct.get_myList().size(); x++) {
            assertEquals(myListStruct.get_myList().get(x), returnStruct.get_myList().get(x));
        }
    }

    @Test
    public void runsendGapAdvertisingData() {
        gapAdvertisingData_t ad = new gapAdvertisingData_t(new ArrayList<>());
        int result;

        for (int x = 0; x < 2; x++) {
            ad.get_aAdStructures().add(new gapAdStructure_t((short) 5, List.of((short) 5, (short) 10, (short) 15)));
        }

        result = client.sendGapAdvertisingData(ad);
        assertEquals(33, result);
    }

    @AfterAll
    public void quit() {
        new CommonClient(clientManager).quit();
    }
}
