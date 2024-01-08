package io.github.embeddedrpc.erpc.tests.server.services;


import io.github.embeddedrpc.erpc.auxiliary.Reference;
import io.github.embeddedrpc.erpc.tests.test_builtin.erpc_outputs.test.server.AbstractBuiltinServicesService;
import org.junit.jupiter.api.Test;

public class TestBuiltinService extends AbstractBuiltinServicesService {
    private int int32A = 2;
    private int int32B = -20;
    private String stringA = "Hello";
    private String stringB = "World!";

    @Override
    public void test_int32_in(int a) {
        System.out.println("test_int32_in reached");
        int32A = a;
    }

    @Override
    public void test_int32_in2(int b) {
        System.out.println("test_int32_in2 reached");
        int32B = b;
    }

    @Override
    public void test_int32_out(Reference<Integer> c) {
        System.out.println("test_int32_out reached");
        c.set(int32A);
    }

    @Override
    public void test_int32_inout(Reference<Integer> e) {
        System.out.println("test_int32_inout reached");
        e.set(e.get() + 1);
    }

    @Override
    public int test_int32_return() {
        System.out.println("test_int32_return reached");
        return int32A * int32B;
    }

    @Override
    public int test_int32_allDirection(int a, int b, Reference<Integer> c, Reference<Integer> e) {
        System.out.println("test_int32_allDirection reached");
        c.set(a);
        e.set(2 * e.get());
        return a * b;
    }

    @Override
    public void test_float_inout(float a, Reference<Float> b) {
        System.out.println("test_float_inout reached");
        b.set(a);
    }

    @Override
    public void test_double_inout(double a, Reference<Double> b) {
        System.out.println("test_double_inout reached");
        b.set(a);
    }

    @Override
    public void test_string_in(String a) {
        System.out.println("test_string_in reached");
        stringA = a;
    }

    @Override
    public void test_string_in2(String b) {
        System.out.println("test_string_in2 reached");
        stringB = b;
    }

    @Override
    public void test_string_out(Reference<String> c) {
        System.out.println("test_string_out reached");
        c.set(stringA);
    }

    @Override
    public void test_string_inout(Reference<String> e) {
        System.out.println("test_string_inout reached");
        e.set(e.get() + stringB);
    }

    @Override
    public String test_string_return() {
        System.out.println("test_string_return reached");
        return stringA + " " + stringB;
    }

    @Override
    public String test_string_allDirection(String a, String b, Reference<String> c, Reference<String> e) {
        System.out.println("test_string_allDirection reached");
        c.set(a);
        e.set(e.get() + stringB);
        return e.get();
    }

    @Override
    public String test_string_empty(String a, String b, Reference<String> c, Reference<String> e) {
        System.out.println("test_string_empty reached");
        return "";
    }

    @Override
    public int sendHello(String str) {
        System.out.println("sendHello reached");
        if (str.equals("Hello World!")) {
            return 0;
        }
        return -1;
    }

    @Override
    public int sendTwoStrings(String myStr1, String myStr2) {
        System.out.println("sendTwoStrings reached");
        if (myStr1.equals("String one.") && myStr2.equals("String two.")) {
            return 0;
        }
        return -1;
    }

    @Override
    public String returnHello() {
        System.out.println("returnHello reached");
        return "Hello";
    }
}
