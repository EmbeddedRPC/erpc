package com.nxp.erpc.tests.client;

import com.nxp.erpc.tests.TestingClient;
import com.nxp.erpc.tests.test_arrays.test_unit_test_common.client.CommonClient;
import com.nxp.erpc.tests.test_typedef.erpc_outputs.test.client.TypedefServiceClient;
import com.nxp.erpc.tests.test_typedef.erpc_outputs.test.common.enums.enumColor;
import com.nxp.erpc.tests.test_typedef.erpc_outputs.test.common.structs.A;
import com.nxp.erpc.tests.test_typedef.erpc_outputs.test.interfaces.ITypedefService;
import org.junit.jupiter.api.AfterAll;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.TestInstance;

import java.util.ArrayList;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;

@TestInstance(TestInstance.Lifecycle.PER_CLASS)
public class TestTypedef extends TestingClient {
    private final TypedefServiceClient client = new TypedefServiceClient(clientManager);

    @Test
    public void runSendReceiveInt() {
        int a = 10;
        int b = 2 * a + 1;
        assertEquals(b, client.sendReceiveInt(a));
    }

    @Test
    public void runTestTypedefServiceID() {
        assertEquals(2, ITypedefService.SERVICE_ID);
    }

    @Test
    public void runSendReceiveEnum() {
        // Java have not typedef
        enumColor a = enumColor.green;

        assertEquals(enumColor.blue, client.sendReceiveEnum(a));
    }

    @Test
    public void runSendReceiveStruct() {
        A a = new A(1, 4);
        A b = new A(2 * a.get_m(), 2 + a.get_n());
        A r;

        r = client.sendReceiveStruct(a);

        assertEquals(b.get_m(), r.get_m());
        assertEquals(b.get_n(), r.get_n());
    }

    @Test
    public void runSendReceiveListType() {
        List<Integer> sendList = new ArrayList<>();

        for (int i = 0; i < 12; i++) {
            sendList.add(i);
        }

        List<Integer> receivedList = client.sendReceiveListType(sendList);

        for (int x = 0; x < receivedList.size(); x++) {
            assertEquals(sendList.get(x), receivedList.get(x) / 2);
        }
    }

    @Test
    public void runSendReceiveString() {
        String send = "Hello";

        String received = client.sendReceiveString(send);

        assertEquals("Hello World!", received);
    }

    @Test
    public void runSendReceive2ListType() {
        List<List<Integer>> sendList = new ArrayList<>();

        for (int i = 0; i < 10; i++) {
            List<Integer> tmp = new ArrayList<>();
            for (int j = 0; j < i + 1; j++) {
                tmp.add(j);
            }
            sendList.add(tmp);
        }

        List<List<Integer>> receivedList = client.sendReceive2ListType(sendList);

        for (int x = 0; x < receivedList.size(); x++) {
            for (int y = 0; y < receivedList.get(0).size(); y++) {
                assertEquals(sendList.get(x).get(y), receivedList.get(x).get(y) / 2);
            }
        }
    }

    @AfterAll
    public void quit() {
        new CommonClient(clientManager).quit();
    }
}
