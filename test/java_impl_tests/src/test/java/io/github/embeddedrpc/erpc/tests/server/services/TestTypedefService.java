package io.github.embeddedrpc.erpc.tests.server.services;

import io.github.embeddedrpc.erpc.tests.test_typedef.erpc_outputs.test.common.enums.enumColor;
import io.github.embeddedrpc.erpc.tests.test_typedef.erpc_outputs.test.common.structs.A;
import io.github.embeddedrpc.erpc.tests.test_typedef.erpc_outputs.test.server.AbstractTypedefServiceService;

import java.util.ArrayList;
import java.util.List;

public class TestTypedefService extends AbstractTypedefServiceService {
    @Override
    public int sendReceiveInt(int a) {
        return a * 2 + 1;
    }

    @Override
    public enumColor sendReceiveEnum(enumColor a) {
        return enumColor.get(a.getValue() + 1);
    }

    @Override
    public A sendReceiveStruct(A a) {
        return new A(2 * a.get_m(), 2 + a.get_n());
    }

    @Override
    public List<Integer> sendReceiveListType(List<Integer> listNumbers) {
        List<Integer> sendList = new ArrayList<>();

        for (Integer listNumber : listNumbers) {
            sendList.add(2 * listNumber);
        }

        return sendList;
    }

    @Override
    public List<List<Integer>> sendReceive2ListType(List<List<Integer>> listNumbers) {
        List<List<Integer>> sendList = new ArrayList<>();

        for (int x = 0; x < listNumbers.size(); x++) {
            List<Integer> tmp = new ArrayList<>();
            for (int y = 0; y < listNumbers.get(x).size(); y++) {
                tmp.add(2 * listNumbers.get(x).get(y));
            }
            sendList.add(tmp);
        }

        return sendList;
    }

    @Override
    public String sendReceiveString(String hello) {
        return hello + " World!";
    }
}
