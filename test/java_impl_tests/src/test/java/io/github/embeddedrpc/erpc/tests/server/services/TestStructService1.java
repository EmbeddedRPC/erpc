package io.github.embeddedrpc.erpc.tests.server.services;

import io.github.embeddedrpc.erpc.tests.test_struct.erpc_outputs.test_ArithmeticService.common.structs.*;
import io.github.embeddedrpc.erpc.tests.test_struct.erpc_outputs.test_ArithmeticService.server.AbstractArithmeticService1Service;

public class TestStructService1 extends AbstractArithmeticService1Service {
    @Override
    public int getMember(C c) {
        System.out.println("getMember reached");
        return c.get_m();
    }

    @Override
    public B returnStruct(float a, float b) {
        System.out.println("returnStruct reached");
        return new B(a, b);
    }

    @Override
    public B getMemberTest2(A a) {
        System.out.println("getMemberTest2 reached");
        return new B(a.get_c().get_m(), a.get_b().get_x());
    }

    @Override
    public int sendNestedStruct(D d) {
        System.out.println("sendNestedStruct reached");
        return d.get_c().get_m();
    }

    @Override
    public int checkString(primate p) {
        System.out.println("checkString reached");
        return !(p.get_species().equals("Ape")) ? 1 : 0;
    }

    @Override
    public stringStruct returnStrings() {
        System.out.println("returnStrings reached");
        return new stringStruct("One", "Two", "Three");
    }
}
