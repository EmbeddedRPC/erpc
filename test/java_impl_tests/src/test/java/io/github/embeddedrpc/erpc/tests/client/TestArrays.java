/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

package io.github.embeddedrpc.erpc.tests.client;

import io.github.embeddedrpc.erpc.auxiliary.Reference;
import io.github.embeddedrpc.erpc.tests.TestingClient;
import io.github.embeddedrpc.erpc.tests.test_arrays.test.client.PointersServiceClient;
import io.github.embeddedrpc.erpc.tests.test_arrays.test.common.enums.enumColor;
import io.github.embeddedrpc.erpc.tests.test_arrays.test.common.structs.AllTypes;
import io.github.embeddedrpc.erpc.tests.test_arrays.test.common.structs.C;
import io.github.embeddedrpc.erpc.tests.test_arrays.test.interfaces.IPointersService;
import io.github.embeddedrpc.erpc.tests.test_arrays.test_unit_test_common.client.CommonClient;
import org.junit.jupiter.api.*;

import java.util.List;

import static org.junit.jupiter.api.Assertions.*;

@TestInstance(TestInstance.Lifecycle.PER_CLASS)
public class TestArrays extends TestingClient {
    private final PointersServiceClient client = new PointersServiceClient(clientManager);

    @Test
    public void runTestArraysServiceID() {
        assertEquals(2, IPointersService.SERVICE_ID);
    }

    @Test
    public void runTestsendReceivedInt32() {
        int[] sendArray = new int[12];
        int[] receivedArray;

        for (int i = 0; i < 12; i++) {
            sendArray[i] = i;
        }

        receivedArray = client.sendReceivedInt32(sendArray);

        for (int i = 0; i < 12; i++) {
            assertEquals(receivedArray[i], sendArray[i]);
        }
    }

    @Test
    public void runTestsendReceived2Int32() {
        int[][] sendArray = new int[12][10];
        int[][] receivedArray;

        for (int a = 0; a < 12; a++) {
            for (int b = 0; b < 10; b++) {
                sendArray[a][b] = a * b;
            }
        }

        receivedArray = client.sendReceived2Int32(sendArray);

        for (int a = 0; a < 12; a++) {
            for (int b = 0; b < 10; b++) {
                assertEquals(sendArray[a][b], receivedArray[a][b]);
            }
        }
    }

    @Test
    public void runTestsendReceivedString() {
        String[] sendArray = new String[12];
        String[] receiveArray;

        for (int i = 0; i < 12; i++) {
            sendArray[i] = "Hello";
        }

        receiveArray = client.sendReceivedString(sendArray);

        for (int i = 0; i < 12; i++) {
            assertEquals(sendArray[i], receiveArray[i]);
        }
    }

    @Test
    public void runTestsendReceived2String() {
        String[][] sendArray = new String[3][5];
        String[][] receiveArray;

        for (int a = 0; a < 3; a++) {
            for (int b = 0; b < 5; b++) {
                sendArray[a][b] = "Hello";
            }
        }

        receiveArray = client.sendReceived2String(sendArray);

        for (int a = 0; a < 3; a++) {
            for (int b = 0; b < 5; b++) {
                assertEquals(sendArray[a][b], receiveArray[a][b]);
            }
        }
    }

    @Test
    public void runTestsendReceivedEnum() {
        enumColor[] sendArray = new enumColor[3];
        enumColor[] receiveArray;

        sendArray[0] = enumColor.red;
        sendArray[1] = enumColor.green;
        sendArray[2] = enumColor.blue;

        receiveArray = client.sendReceivedEnum(sendArray);

        for (int i = 0; i < 3; i++) {
            assertEquals(sendArray[i], receiveArray[i]);
        }
    }

    @Test
    public void runTestsendReceived2Enum() {
        enumColor[][] sendArray = new enumColor[3][3];
        enumColor[][] receiveArray;

        for (int a = 0; a < 3; a++) {
            sendArray[a][0] = enumColor.red;
            sendArray[a][1] = enumColor.green;
            sendArray[a][2] = enumColor.blue;
        }

        receiveArray = client.sendReceived2Enum(sendArray);

        for (int a = 0; a < 3; a++) {
            for (int b = 0; b < 3; b++) {
                assertEquals(sendArray[a][b], receiveArray[a][b]);
            }
        }
    }

    @Test
    public void runTestsendReceivedList() {
        List<Integer>[] sendArray = new List[]{List.of(0, 1), List.of(0, 1)};
        List<Integer>[] receiveArray;

        receiveArray = client.sendReceivedList(sendArray);

        assertEquals(sendArray[0], receiveArray[0]);
        assertEquals(sendArray[1], receiveArray[1]);
    }

    @Test
    public void runTestsendReceived2List() {
        List<Integer>[][] sendArray = new List[][]{
                {List.of(0, 1), List.of(0, 1)},
                {List.of(0, 1), List.of(0, 1)}
        };
        List<Integer>[][] receiveArray;

        receiveArray = client.sendReceived2List(sendArray);

        assertEquals(sendArray[0][0], receiveArray[0][0]);
        assertEquals(sendArray[0][1], receiveArray[0][1]);
        assertEquals(sendArray[1][0], receiveArray[1][0]);
        assertEquals(sendArray[1][1], receiveArray[1][1]);
    }

    @Test
    public void runTestsendReceivedInt32Type() {
        int[] sendArray = new int[12];
        int[] receivedArray;

        for (int i = 0; i < 12; i++) {
            sendArray[i] = i;
        }

        receivedArray = client.sendReceivedInt32(sendArray);

        for (int i = 0; i < 12; i++) {
            assertEquals(receivedArray[i], sendArray[i]);
        }
    }

    @Test
    public void runTestsendReceived2Int32Type() {
        int[][] sendArray = new int[12][10];
        int[][] receivedArray;

        for (int a = 0; a < 12; a++) {
            for (int b = 0; b < 10; b++) {
                sendArray[a][b] = a * b;
            }
        }

        receivedArray = client.sendReceived2Int32(sendArray);

        for (int a = 0; a < 12; a++) {
            for (int b = 0; b < 10; b++) {
                assertEquals(sendArray[a][b], receivedArray[a][b]);
            }
        }
    }

    @Test
    public void runTestsendReceivedStringType() {
        String[] sendArray = new String[12];
        String[] receiveArray;

        for (int i = 0; i < 12; i++) {
            sendArray[i] = "Hello";
        }

        receiveArray = client.sendReceivedString(sendArray);

        for (int i = 0; i < 12; i++) {
            assertEquals(sendArray[i], receiveArray[i]);
        }
    }

    @Test
    public void runTestsendReceived2StringType() {
        String[][] sendArray = new String[3][5];
        String[][] receiveArray;

        for (int a = 0; a < 3; a++) {
            for (int b = 0; b < 5; b++) {
                sendArray[a][b] = "Hello";
            }
        }

        receiveArray = client.sendReceived2String(sendArray);

        for (int a = 0; a < 3; a++) {
            for (int b = 0; b < 5; b++) {
                assertEquals(sendArray[a][b], receiveArray[a][b]);
            }
        }
    }

    @Test
    public void runTestsendReceivedEnumType() {
        enumColor[] sendArray = new enumColor[3];
        enumColor[] receiveArray;

        sendArray[0] = enumColor.red;
        sendArray[1] = enumColor.green;
        sendArray[2] = enumColor.blue;

        receiveArray = client.sendReceivedEnum(sendArray);

        for (int i = 0; i < 3; i++) {
            assertEquals(sendArray[i], receiveArray[i]);
        }
    }

    @Test
    public void runTestsendReceived2EnumType() {
        enumColor[][] sendArray = new enumColor[3][3];
        enumColor[][] receiveArray;

        for (int a = 0; a < 3; a++) {
            sendArray[a][0] = enumColor.red;
            sendArray[a][1] = enumColor.green;
            sendArray[a][2] = enumColor.blue;
        }

        receiveArray = client.sendReceived2Enum(sendArray);

        for (int a = 0; a < 3; a++) {
            for (int b = 0; b < 3; b++) {
                assertEquals(sendArray[a][b], receiveArray[a][b]);
            }
        }
    }

    @Test
    public void runTestsendReceivedStructType() {
        C[] sendArray = new C[3];
        C[] receiveArray;

        for (int a = 0; a < 3; a++) {
            sendArray[a] = new C(a, a + 2);
        }

        receiveArray = client.sendReceivedStructType(sendArray);

        for (int x = 0; x < 3; x++) {
            assertEquals(sendArray[x].get_m(), receiveArray[x].get_m());
            assertEquals(sendArray[x].get_n(), receiveArray[x].get_n());
        }
    }

    @Test
    public void runTestsendReceived2StructType() {
        C[][] sendArray = new C[3][3];
        C[][] receiveArray;

        for (int a = 0; a < 3; a++) {
            for (int b = 0; b < 3; b++) {
                sendArray[a][b] = new C(a, b);
            }
        }

        receiveArray = client.sendReceived2StructType(sendArray);

        for (int a = 0; a < 3; a++) {
            for (int b = 0; b < 3; b++) {
                assertEquals(sendArray[a][b].get_m(), receiveArray[a][b].get_m());
                assertEquals(sendArray[a][b].get_n(), receiveArray[a][b].get_n());
            }
        }
    }

    @Test
    public void runTestsendReceivedListType() {
        List<Integer>[] sendArray = new List[]{List.of(0, 1), List.of(0, 1)};
        List<Integer>[] receiveArray;

        receiveArray = client.sendReceivedList(sendArray);

        assertEquals(sendArray[0], receiveArray[0]);
        assertEquals(sendArray[1], receiveArray[1]);
    }

    @Test
    public void runTestsendReceived2ListType() {
        List<Integer>[][] sendArray = new List[][]{
                {List.of(0, 1), List.of(0, 1)},
                {List.of(0, 1), List.of(0, 1)}
        };
        List<Integer>[][] receiveArray;

        receiveArray = client.sendReceived2List(sendArray);

        assertEquals(sendArray[0][0], receiveArray[0][0]);
        assertEquals(sendArray[0][1], receiveArray[0][1]);
        assertEquals(sendArray[1][0], receiveArray[1][0]);
        assertEquals(sendArray[1][1], receiveArray[1][1]);
    }

    @Test
    public void runTestsendReceiveStruct() {
        AllTypes[] sendArray = new AllTypes[2];
        AllTypes[] receiveArray;

        for (int i = 0; i < 2; i++) {
            sendArray[i] = new AllTypes(
                    5,
                    "Hello",
                    new C(5, 20),
                    enumColor.red,
                    List.of(0, 1, 2, 3, 4),
                    List.of("Hello", "Hello", "Hello", "Hello", "Hello"),
                    new int[]{0, 1, 2, 3, 4},
                    new String[]{"Hello", "Hello", "Hello", "Hello", "Hello"}
            );
        }


        receiveArray = client.sendReceiveStruct(sendArray);

        for (int i = 0; i < 2; i++) {
            assertEquals(receiveArray[i].get_number(), sendArray[i].get_number());
            assertEquals(receiveArray[i].get_text(), sendArray[i].get_text());
            assertEquals(receiveArray[i].get_color(), sendArray[i].get_color());
            assertEquals(receiveArray[i].get_c().get_m(), sendArray[i].get_c().get_m());
            assertEquals(receiveArray[i].get_c().get_n(), sendArray[i].get_c().get_n());
            assertEquals(receiveArray[i].get_list_numbers(), sendArray[i].get_list_numbers());
            assertEquals(receiveArray[i].get_list_text(), sendArray[i].get_list_text());
            assertArrayEquals(receiveArray[i].get_array_numbers(), sendArray[i].get_array_numbers());
            assertArrayEquals(receiveArray[i].get_array_text(), sendArray[i].get_array_text());
        }

    }

    @Test
    public void runTestsendReceive2Struct() {
        AllTypes[][] sendArray = new AllTypes[1][1];
        AllTypes[][] receiveArray;

        sendArray[0][0] = new AllTypes(
                5,
                "Hello",
                new C(5, 20),
                enumColor.red,
                List.of(0, 1, 2, 3, 4),
                List.of("Hello", "Hello", "Hello", "Hello", "Hello"),
                new int[]{0, 1, 2, 3, 4},
                new String[]{"Hello", "Hello", "Hello", "Hello", "Hello"}
        );

        receiveArray = client.sendReceive2Struct(sendArray);

        assertEquals(receiveArray[0][0].get_number(), sendArray[0][0].get_number());
        assertEquals(receiveArray[0][0].get_text(), sendArray[0][0].get_text());
        assertEquals(receiveArray[0][0].get_color(), sendArray[0][0].get_color());
        assertEquals(receiveArray[0][0].get_c().get_m(), sendArray[0][0].get_c().get_m());
        assertEquals(receiveArray[0][0].get_c().get_n(), sendArray[0][0].get_c().get_n());
        assertEquals(receiveArray[0][0].get_list_numbers(), sendArray[0][0].get_list_numbers());
        assertEquals(receiveArray[0][0].get_list_text(), sendArray[0][0].get_list_text());
        assertArrayEquals(receiveArray[0][0].get_array_numbers(), sendArray[0][0].get_array_numbers());
        assertArrayEquals(receiveArray[0][0].get_array_text(), sendArray[0][0].get_array_text());
    }

    @Test
    public void runTesttest_array_allDirection() {
        int[] aArray = new int[5];
        int[] bArray = new int[5];
        Reference<int[]> cArray = new Reference<>(new int[5]);
        Reference<int[]> dArray = new Reference<>(new int[5]);

        int[] pAArray = new int[5];
        int[] pBArray = new int[5];
        int[] pCArray = new int[5];
        int[] pDArray = new int[5];

        for (int a = 0; a < 5; a++) {
            aArray[a] = a;
            bArray[a] = a * 2;
            dArray.get()[a] = a * 4;

            pAArray[a] = aArray[a];
            pBArray[a] = bArray[a];
            pCArray[a] = a + 1;
            pDArray[a] = 2 * dArray.get()[a];
        }

        client.test_array_allDirection(aArray, bArray, cArray, dArray);

        for (int a = 0; a < 5; a++) {
            assertEquals(aArray[a], pAArray[a]);
            assertEquals(bArray[a], pBArray[a]);
            assertEquals(cArray.get()[a], pCArray[a]);
            assertEquals(dArray.get()[a], pDArray[a]);
        }
    }

    @AfterAll
    public void quit() {
        new CommonClient(clientManager).quit();
    }
}
