package com.nxp.erpc.tests.server.services;

import com.nxp.erpc.auxiliary.Reference;
import com.nxp.erpc.tests.test_lists.erpc_outputs.test.common.enums.enumColor;
import com.nxp.erpc.tests.test_lists.erpc_outputs.test.common.structs.C;
import com.nxp.erpc.tests.test_lists.erpc_outputs.test.common.structs.gapAdStructure_t;
import com.nxp.erpc.tests.test_lists.erpc_outputs.test.common.structs.gapAdvertisingData_t;
import com.nxp.erpc.tests.test_lists.erpc_outputs.test.common.structs.listStruct;
import com.nxp.erpc.tests.test_lists.erpc_outputs.test.server.AbstractPointersServiceService;

import java.util.ArrayList;
import java.util.List;

public class TestListsService extends AbstractPointersServiceService {
    @Override
    public List<Integer> sendReceivedInt32(List<Integer> listNumbers) {
        System.out.println("sendReceivedInt32 reached");
        List<Integer> sendList = new ArrayList<>();

        for (int i = 0; i < listNumbers.size(); i++) {
            sendList.add(2 * listNumbers.get(i));
        }

        return sendList;
    }

    @Override
    public List<List<Integer>> sendReceived2Int32(List<List<Integer>> listNumbers) {
        System.out.println("sendReceived2Int32 reached");
        List<List<Integer>> sendList = new ArrayList<>();

        for (int x = 0; x < listNumbers.size(); x++) {
            sendList.add(new ArrayList<>());
            for (int y = 0; y < listNumbers.get(x).size(); y++) {
                sendList.get(x).add(2 * listNumbers.get(x).get(y));
            }
        }

        return sendList;
    }

    @Override
    public List<enumColor> sendReceivedEnum(List<enumColor> listColors) {
        System.out.println("sendReceivedEnum reached");
        return new ArrayList<>(listColors);
    }

    @Override
    public List<List<enumColor>> sendReceived2Enum(List<List<enumColor>> listColors) {
        System.out.println("sendReceived2Enum reached");
        return new ArrayList<>(listColors);
    }

    @Override
    public List<C> sendReceivedStruct(List<C> listColors) {
        System.out.println("sendReceivedStruct reached");
        List<C> sendList = new ArrayList<>();

        for (C listColor : listColors) {
            sendList.add(new C(2 * listColor.get_m(), 2 * listColor.get_n()));
        }

        return sendList;
    }

    @Override
    public List<List<C>> sendReceived2Struct(List<List<C>> listColors) {
        System.out.println("sendReceived2Struct reached");
        List<List<C>> sendList = new ArrayList<>();

        for (List<C> listColor : listColors) {
            List<C> tmpList = new ArrayList<>();
            for (C c : listColor) {
                tmpList.add(new C(2 * c.get_m(), 2 * c.get_n()));
            }
            sendList.add(tmpList);
        }

        return sendList;
    }

    @Override
    public List<String> sendReceivedString(List<String> listNumbers) {
        System.out.println("sendReceivedString reached");
        List<String> sendList = new ArrayList<>();

        for (String s : listNumbers) {
            sendList.add(s + " World!");
        }

        return sendList;
    }

    @Override
    public List<List<String>> sendReceived2String(List<List<String>> listNumbers) {
        System.out.println("sendReceived2String reached");
        List<List<String>> sendList = new ArrayList<>();

        for (List<String> strings : listNumbers) {
            List<String> tmpList = new ArrayList<>();
            for (String string : strings) {
                tmpList.add(string + " World!");
            }
            sendList.add(tmpList);
        }

        return sendList;
    }

    @Override
    public void test_list_allDirection(List<Long> a, List<Long> b, Reference<List<Long>> e) {
        System.out.println("test_list_allDirection reached");
        for (int i = 0; i < a.size(); i++) {
            e.get().set(i, 2 * e.get().get(i));
        }
    }

    @Override
    public int testLengthAnnotation(List<Integer> myList) {
        System.out.println("testLengthAnnotation reached");
        for (int i = 0; i < myList.size(); i++) {
            if (myList.get(i) != i + 1) {
                return 0;
            }
        }

        return 1;
    }

    @Override
    public int testLengthAnnotationInStruct(listStruct s) {
        System.out.println("testLengthAnnotationInStruct reached");
        for (int i = 0; i < s.get_myList().size(); i++) {
            if (s.get_myList().get(i) != i + 1) {
                return 0;
            }
        }

        return 10;
    }

    @Override
    public listStruct returnSentStructLengthAnnotation(listStruct s) {
        System.out.println("returnSentStructLengthAnnotation reached");
        listStruct ret = new listStruct(new ArrayList<>());

        for (int i = 0; i < s.get_myList().size(); i++) {
            ret.get_myList().add(s.get_myList().get(i));
        }

        return ret;
    }

    @Override
    public int sendGapAdvertisingData(gapAdvertisingData_t ad) {
        System.out.println("sendGapAdvertisingData reached");
        List<gapAdStructure_t> gapAdStructureTs = ad.get_aAdStructures();

        for (gapAdStructure_t gapAdStructureT : gapAdStructureTs) {
            if (gapAdStructureT.get_adType() != 5) {
                return 0;
            }

            for (int y = 0; y < gapAdStructureT.length(); y++) {
                if (gapAdStructureT.get_aData().get(y) != ((y + 1) * 5)) {
                    return 0;
                }
            }
        }

        return 33;
    }
}
