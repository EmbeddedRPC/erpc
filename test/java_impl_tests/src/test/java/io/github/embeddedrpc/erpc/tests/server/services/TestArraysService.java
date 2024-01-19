package io.github.embeddedrpc.erpc.tests.server.services;

import io.github.embeddedrpc.erpc.auxiliary.Reference;
import io.github.embeddedrpc.erpc.tests.test_arrays.test.common.enums.enumColor;
import io.github.embeddedrpc.erpc.tests.test_arrays.test.common.structs.AllTypes;
import io.github.embeddedrpc.erpc.tests.test_arrays.test.common.structs.C;
import io.github.embeddedrpc.erpc.tests.test_arrays.test.server.AbstractPointersServiceService;
import io.github.embeddedrpc.erpc.tests.test_struct.erpc_outputs.test_ArithmeticService.common.structs.A;

import java.util.List;

public class TestArraysService extends AbstractPointersServiceService {
    @Override
    public int[] sendReceivedInt32(int[] arrayNumbers) {
        System.out.println("sendReceivedInt32 reached");
        int[] sendArray = new int[12];

        System.arraycopy(arrayNumbers, 0, sendArray, 0, 12);

        return sendArray;
    }

    @Override
    public int[][] sendReceived2Int32(int[][] arrayNumbers) {
        System.out.println("sendReceived2Int32 reached");
        int[][] sendArray = new int[12][10];

        for (int y = 0; y < 12; y++) {
            System.arraycopy(arrayNumbers[y], 0, sendArray[y], 0, 10);
        }

        return sendArray;
    }

    @Override
    public String[] sendReceivedString(String[] arrayStrings) {
        System.out.println("sendReceivedString reached");
        String[] sendArray = new String[12];

        System.arraycopy(arrayStrings, 0, sendArray, 0, 12);

        return sendArray;
    }

    @Override
    public String[][] sendReceived2String(String[][] arrayStrings) {
        System.out.println("sendReceived2String reached");
        String[][] sendArray = new String[3][5];

        for (int y = 0; y < 3; y++) {
            System.arraycopy(arrayStrings[y], 0, sendArray[y], 0, 5);
        }

        return sendArray;
    }

    @Override
    public enumColor[] sendReceivedEnum(enumColor[] arrayEnums) {
        System.out.println("sendReceivedEnum reached");
        enumColor[] sendArray = new enumColor[3];

        System.arraycopy(arrayEnums, 0, sendArray, 0, 3);

        return sendArray;
    }

    @Override
    public enumColor[][] sendReceived2Enum(enumColor[][] arrayEnums) {
        System.out.println("sendReceived2Enum reached");
        enumColor[][] sendArray = new enumColor[3][3];

        for (int y = 0; y < 3; y++) {
            System.arraycopy(arrayEnums[y], 0, sendArray[y], 0, 3);
        }

        return sendArray;
    }

    @Override
    public List<Integer>[] sendReceivedList(List<Integer>[] arrayLists) {
        System.out.println("sendReceivedList reached");
        List<Integer>[] sendArray = new List[2];

        System.arraycopy(arrayLists, 0, sendArray, 0, 2);

        return sendArray;
    }

    @Override
    public List<Integer>[][] sendReceived2List(List<Integer>[][] arrayLists) {
        System.out.println("sendReceived2List reached");
        List<Integer>[][] sendArray = new List[2][2];

        for (int y = 0; y < 2; y++) {
            System.arraycopy(arrayLists[y], 0, sendArray[y], 0, 2);
        }

        return sendArray;
    }

    @Override
    public int[] sendReceivedInt32Type(int[] arrayNumbers) {
        System.out.println("sendReceivedInt32Type reached");
        int[] sendArray = new int[12];

        System.arraycopy(arrayNumbers, 0, sendArray, 0, 12);

        return sendArray;
    }

    @Override
    public int[][] sendReceived2Int32Type(int[][] arrayNumbers) {
        System.out.println("sendReceived2Int32Type reached");
        int[][] sendArray = new int[12][10];

        for (int i = 0; i < 12; i++) {
            for (int j = 0; j < 10; j++) {
                sendArray[i][j] = arrayNumbers[i][j];
            }
        }

        return sendArray;
    }

    @Override
    public String[] sendReceivedStringType(String[] arrayStrings) {
        System.out.println("sendReceivedStringType reached");
        String[] sendArray = new String[12];

        System.arraycopy(arrayStrings, 0, sendArray, 0, 12);

        return sendArray;
    }

    @Override
    public String[][] sendReceived2StringType(String[][] arrayStrings) {
        System.out.println("sendReceived2StringType reached");
        String[][] sendArray = new String[3][5];

        for (int y = 0; y < 3; y++) {
            System.arraycopy(arrayStrings[y], 0, sendArray[y], 0, 5);
        }

        return sendArray;
    }

    @Override
    public enumColor[] sendReceivedEnumType(enumColor[] arrayEnums) {
        System.out.println("sendReceivedEnumType reached");
        enumColor[] sendArray = new enumColor[3];

        System.arraycopy(arrayEnums, 0, sendArray, 0, 3);

        return sendArray;
    }

    @Override
    public enumColor[][] sendReceived2EnumType(enumColor[][] arrayEnums) {
        System.out.println("sendReceived2EnumType reached");
        enumColor[][] sendArray = new enumColor[3][3];

        for (int y = 0; y < 3; y++) {
            System.arraycopy(arrayEnums[y], 0, sendArray[y], 0, 3);
        }

        return sendArray;
    }

    @Override
    public C[] sendReceivedStructType(C[] arrayStructs) {
        System.out.println("sendReceivedStructType reached");
        C[] sendArrays = new C[]{new C(), new C(), new C()};

        for (int i = 0; i < 3; i++) {
            sendArrays[i].set_m(arrayStructs[i].get_m());
            sendArrays[i].set_n(arrayStructs[i].get_n());
        }

        return sendArrays;
    }

    @Override
    public C[][] sendReceived2StructType(C[][] arrayStructs) {
        System.out.println("sendReceived2StructType reached");
        C[][] sendArrays = new C[][]{
                {new C(), new C(), new C()},
                {new C(), new C(), new C()},
                {new C(), new C(), new C()}
        };

        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                sendArrays[i][j].set_m(arrayStructs[i][j].get_m());
                sendArrays[i][j].set_n(arrayStructs[i][j].get_n());
            }
        }

        return sendArrays;
    }

    @Override
    public List<Integer>[] sendReceivedListType(List<Integer>[] arrayLists) {
        System.out.println("sendReceivedListType reached");
        List<Integer>[] sendArray = new List[2];

        System.arraycopy(arrayLists, 0, sendArray, 0, 2);

        return sendArray;
    }

    @Override
    public List<Integer>[][] sendReceived2ListType(List<Integer>[][] arrayLists) {
        System.out.println("sendReceived2ListType reached");
        List<Integer>[][] sendArray = new List[2][2];

        for (int y = 0; y < 2; y++) {
            System.arraycopy(arrayLists[y], 0, sendArray[y], 0, 2);
        }

        return sendArray;
    }

    @Override
    public AllTypes[] sendReceiveStruct(AllTypes[] arrayStructs) {
        System.out.println("sendReceiveStruct reached");
        AllTypes[] sendArrays = new AllTypes[]{new AllTypes(), new AllTypes()};

        for (int y = 0; y < 2; y++) {
            sendArrays[y].set_c(new C());
            sendArrays[y].set_number(arrayStructs[y].get_number());
            sendArrays[y].set_text(arrayStructs[y].get_text());
            sendArrays[y].set_color(arrayStructs[y].get_color());
            sendArrays[y].set_list_numbers(arrayStructs[y].get_list_numbers());
            sendArrays[y].set_list_text(arrayStructs[y].get_list_text());
            sendArrays[y].set_array_numbers(arrayStructs[y].get_array_numbers());
            sendArrays[y].set_array_text(arrayStructs[y].get_array_text());
            sendArrays[y].get_c().set_m(arrayStructs[y].get_c().get_m());
            sendArrays[y].get_c().set_n(arrayStructs[y].get_c().get_n());
        }

        return sendArrays;
    }

    @Override
    public AllTypes[][] sendReceive2Struct(AllTypes[][] arrayStructs) {
        System.out.println("sendReceive2Struct reached");
        AllTypes[][] sendArrays = new AllTypes[][]{{new AllTypes()}};

        for (int y = 0; y < 1; y++) {
            for (int z = 0; z < 1; z++) {
                sendArrays[y][z].set_c(new C());
                sendArrays[y][z].set_number(arrayStructs[y][z].get_number());
                sendArrays[y][z].set_text(arrayStructs[y][z].get_text());
                sendArrays[y][z].set_color(arrayStructs[y][z].get_color());
                sendArrays[y][z].set_list_numbers(arrayStructs[y][z].get_list_numbers());
                sendArrays[y][z].set_list_text(arrayStructs[y][z].get_list_text());
                sendArrays[y][z].set_array_numbers(arrayStructs[y][z].get_array_numbers());
                sendArrays[y][z].set_array_text(arrayStructs[y][z].get_array_text());
                sendArrays[y][z].get_c().set_m(arrayStructs[y][z].get_c().get_m());
                sendArrays[y][z].get_c().set_n(arrayStructs[y][z].get_c().get_n());
            }
        }

        return sendArrays;
    }

    @Override
    public void test_array_allDirection(int[] a, int[] b, Reference<int[]> c, Reference<int[]> d) {
        System.out.println("test_array_allDirection reached");
        c.set(new int[5]);

        for (int i = 0; i < 5; i++) {
            c.get()[i] = i + 1;
            d.get()[i] = i * 8;
        }
    }
}
