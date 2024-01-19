package io.github.embeddedrpc.erpc.tests.server.services;

import io.github.embeddedrpc.erpc.auxiliary.Reference;
import io.github.embeddedrpc.erpc.tests.test_enums.erpc_outputs.test.common.enums.enumColor;
import io.github.embeddedrpc.erpc.tests.test_enums.erpc_outputs.test.common.enums.enumColor2;
import io.github.embeddedrpc.erpc.tests.test_enums.erpc_outputs.test.common.enums.enumErrorCode;
import io.github.embeddedrpc.erpc.tests.test_enums.erpc_outputs.test.server.AbstractEnumsServiceService;

public class TestEnumsService extends AbstractEnumsServiceService {
    private enumColor gEnumColorAServer = null;
    private enumColor gEnumColorBServer = null;

    @Override
    public void test_enumColor_in(enumColor a) {
        System.out.println("test_enumColor_in reached");
        gEnumColorAServer = a;
    }

    @Override
    public void test_enumColor_in2(enumColor b) {
        System.out.println("test_enumColor_in2 reached");
        gEnumColorBServer = b;
    }

    @Override
    public void test_enumColor_out(Reference<enumColor> c) {
        System.out.println("test_enumColor_out reached");
        c.set(gEnumColorAServer);
    }

    @Override
    public void test_enumColor_inout(Reference<enumColor> e) {
        System.out.println("test_enumColor_inout reached");
        e.set(enumColor.get(e.get().getValue() - 9));
    }

    @Override
    public enumColor test_enumColor_return() {
        System.out.println("test_enumColor_return reached");
        return enumColor.get(gEnumColorAServer.getValue() + 9);
    }

    @Override
    public enumColor test_enumColor_allDirection(enumColor a, enumColor b, Reference<enumColor> c, Reference<enumColor> e) {
        System.out.println("test_enumColor_allDirection reached");
        c.set(a);
        e.set(b);
        return a;
    }

    @Override
    public enumColor2 test_enumColor2_allDirection(enumColor2 a, enumColor2 b, Reference<enumColor2> c, Reference<enumColor2> e) {
        System.out.println("test_enumColor2_allDirection reached");
        c.set(a);
        e.set(b);
        return a;
    }

    @Override
    public enumErrorCode test_enumErrorCode_allDirection(enumErrorCode a, enumErrorCode b, Reference<enumErrorCode> c, Reference<enumErrorCode> e) {
        System.out.println("test_enumErrorCode_allDirection reached");
        c.set(a);
        e.set(b);
        return a;
    }
}
