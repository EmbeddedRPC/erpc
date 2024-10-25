/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

package io.github.embeddedrpc.erpc.tests.client;


import io.github.embeddedrpc.erpc.auxiliary.Reference;
import io.github.embeddedrpc.erpc.tests.TestingClient;
import io.github.embeddedrpc.erpc.tests.test_arrays.test_unit_test_common.client.CommonClient;
import io.github.embeddedrpc.erpc.tests.test_struct.erpc_outputs.test_ArithmeticService.client.ArithmeticService1Client;
import io.github.embeddedrpc.erpc.tests.test_struct.erpc_outputs.test_ArithmeticService.client.ArithmeticService2Client;
import io.github.embeddedrpc.erpc.tests.test_struct.erpc_outputs.test_ArithmeticService.common.enums.enumColor;
import io.github.embeddedrpc.erpc.tests.test_struct.erpc_outputs.test_ArithmeticService.common.enums.school_year_t;
import io.github.embeddedrpc.erpc.tests.test_struct.erpc_outputs.test_ArithmeticService.common.structs.*;
import io.github.embeddedrpc.erpc.tests.test_struct.erpc_outputs.test_ArithmeticService.interfaces.IArithmeticService1;
import org.junit.jupiter.api.*;

import java.util.List;

import static org.junit.jupiter.api.Assertions.*;

@TestInstance(TestInstance.Lifecycle.PER_CLASS)
public class TestStruct extends TestingClient {
    private final ArithmeticService1Client client1 = new ArithmeticService1Client(clientManager);
    private final ArithmeticService2Client client2 = new ArithmeticService2Client(clientManager);

    @Test
    public void testArithmeticServiceID() {
        assertEquals(3, IArithmeticService1.SERVICE_ID);
    }

    @Test
    public void getMember1() {
        C c = new C(4, 5);

        int r = client1.getMember(c);

        assertEquals(4, r);
    }

    @Test
    public void nestedStruct1() {
        C c = new C(4, 5);
        D d = new D(c);

        int r = client1.sendNestedStruct(d);

        assertEquals(4, r);
    }

    @Test
    public void returnStruct1() {
        B b;

        b = client1.returnStruct(3.14F, 2.71828F);

        assertEquals(3.14F, b.get_x());
        assertEquals(2.71828F, b.get_y());
    }

    @Test
    public void sendIntSizes() {
        F f = new F((byte) -1, (short) -1, -1, (long) -1);

        assertEquals(0, client2.sendManyInts(f));
    }

    @Test
    public void sendUnsignedIntSizes() {
        // UInt64 not supported
    }

    @Test
    public void getMember2() {
        C c = new C(4, 5);
        B b = new B(3.14F, 2.71828F);
        A a = new A(b, c);
        B bb;

        bb = client1.getMemberTest2(a);

        assertEquals(4.0F, bb.get_x());
        assertEquals(3.14F, bb.get_y());
    }

    @Test
    public void testString1() {
        primate prim8 = new primate("Ape", false);

        assertEquals(0, client1.checkString(prim8));
    }

    @Test
    public void testReturnString1() {
        stringStruct result;

        result = client1.returnStrings();

        assertEquals("One", result.get_one());
        assertEquals("Two", result.get_two());
        assertEquals("Three", result.get_three());
    }

    @Test
    public void testStudent1() {
        student stud = new student("Donnie Darko", new float[]{85.75F, 93.29F, 92.46F}, school_year_t.SENIOR, 19);
        assertEquals("Donnie Darko", client2.getStudentName(stud));
        assertEquals((stud.get_test_grades()[0] + stud.get_test_grades()[1] + stud.get_test_grades()[2]) / 3, client2.getStudentTestAverage(stud));
        assertEquals(19, client2.getStudentAge(stud));
        assertEquals(school_year_t.SENIOR.getValue(), client2.getStudentYear(stud));
    }

    @Test
    public void testStudent2() {
        student stud;

        stud = client2.createStudent("George Bush", new float[]{65.32F, 69.68F, 0}, school_year_t.FRESHMAN, 68);
        assertEquals("George Bush", client2.getStudentName(stud));
        assertEquals((65.32 + 69.68 + 0) / 3, client2.getStudentTestAverage(stud));
        assertEquals(68, client2.getStudentAge(stud));
        assertEquals(school_year_t.FRESHMAN.getValue(), client2.getStudentYear(stud));
    }

    @Test
    public void test_struct_allDirection() {

        byte[] a_binary = new byte[2];
        byte[] b_binary = new byte[2];
        byte[] e_binary = new byte[2];

        for (byte i = 0; i < 2; i++) {
            a_binary[i] = i;
            b_binary[i] = (byte) (i * 2);
            e_binary[i] = i;
        }

        AllTypes a = new AllTypes(5, "Hello", enumColor.red, new C(5, 20), List.of(0, 1), List.of("Hello", "Hello"), new int[]{0, 1}, new String[]{"Hello", "Hello"}, a_binary);
        AllTypes b = new AllTypes(10, "World", enumColor.green, new C(7, 17), List.of(1, 2), List.of("World", "World"), new int[]{1, 2}, new String[]{"World", "World"}, b_binary);
        Reference<AllTypes> e = new Reference<>(new AllTypes(15, "Hello", enumColor.blue, new C(4, 14), List.of(2, 3), List.of("Hello", "Hello"), new int[]{2, 3}, new String[]{"Hello", "Hello"}, e_binary));
        client2.test_struct_allDirection(a, b, e);

        assertEquals(e.get().get_number(), a.get_number() * b.get_number());
        assertEquals(e.get().get_text(), "World");
        assertEquals(e.get().get_color(), enumColor.green);
        assertEquals(e.get().get_c().get_m(), a.get_c().get_m() * b.get_c().get_m());
        assertEquals(e.get().get_c().get_n(), a.get_c().get_n() * b.get_c().get_n());
        assertEquals(2, e.get().get_list_numbers().size());
        for (int i = 0; i < a.get_list_numbers().size(); i++) {
            assertEquals(e.get().get_list_numbers().get(i), a.get_list_numbers().get(i) * b.get_list_numbers().get(i));
            assertEquals("World", e.get().get_list_text().get(i));
        }

        for (int i = 0; i < 2; i++) {
            assertEquals(e.get().get_array_numbers()[i], a.get_array_numbers()[i] * b.get_array_numbers()[i]);
            assertEquals("World", e.get().get_list_text().get(i));
        }

        for (int i = 0; i < a.get_binary_numbers().length; i++) {
            assertEquals(e.get().get_binary_numbers()[i], a.get_binary_numbers()[i] * b.get_binary_numbers()[i]);
        }
    }

    @Test
    public void testSendingByrefMembers() {
        StructWithByrefMembers s;
        int b = 4;
        // Not sure how the test work
    }

    @AfterAll
    public void quit() {
        new CommonClient(clientManager).quit();
    }
}
