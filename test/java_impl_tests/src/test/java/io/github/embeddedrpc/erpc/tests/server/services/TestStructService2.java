package io.github.embeddedrpc.erpc.tests.server.services;

import io.github.embeddedrpc.erpc.auxiliary.Reference;
import io.github.embeddedrpc.erpc.tests.test_struct.erpc_outputs.test_ArithmeticService.common.enums.enumColor;
import io.github.embeddedrpc.erpc.tests.test_struct.erpc_outputs.test_ArithmeticService.common.enums.school_year_t;
import io.github.embeddedrpc.erpc.tests.test_struct.erpc_outputs.test_ArithmeticService.common.structs.*;
import io.github.embeddedrpc.erpc.tests.test_struct.erpc_outputs.test_ArithmeticService.server.AbstractArithmeticService2Service;

import java.util.ArrayList;

public class TestStructService2 extends AbstractArithmeticService2Service {
    @Override
    public int sendManyInts(F f) {
        System.out.println("sendManyInts reached");
        if (f.get_a() == -1 && f.get_b() == -1 && f.get_c() == -1 && f.get_d() == -1) {
            return 0;
        }

        return -1;
    }

    @Override
    public int sendManyUInts(G g) {
        System.out.println("sendManyUInts reached");
        return 0;
    }

    @Override
    public String getStudentName(student stud) {
        System.out.println("getStudentName reached");
        return stud.get_name();
    }

    @Override
    public float getStudentTestAverage(student stud) {
        System.out.println("getStudentTestAverage reached");
        float total = 0;

        for (var testGrade : stud.get_test_grades()) {
            total += testGrade;
        }

        return total / stud.get_test_grades().length;
    }

    @Override
    public int getStudentYear(student stud) {
        System.out.println("getStudentYear reached");
        return stud.get_school_year().getValue();
    }

    @Override
    public int getStudentAge(student stud) {
        System.out.println("getStudentAge reached");
        return stud.get_age();
    }

    @Override
    public student createStudent(String name, float[] test_scores, school_year_t year, int age) {
        System.out.println("createStudent reached");
        return new student(name, test_scores, year, age);
    }

    @Override
    public void test_struct_allDirection(AllTypes a, AllTypes b, Reference<AllTypes> e) {
        System.out.println("test_struct_allDirection reached");
        AllTypes eStruct = e.get();

        eStruct.set_number(a.get_number() * b.get_number());
        eStruct.set_text("World");
        eStruct.set_color(enumColor.green);
        eStruct.set_c(new C(a.get_c().get_m() * b.get_c().get_m(), a.get_c().get_n() * b.get_c().get_n()));

        for (int i = 0; i < a.get_list_numbers().size(); i++) {
            eStruct.get_list_numbers().set(i, a.get_list_numbers().get(i) * b.get_list_numbers().get(i));
            eStruct.get_list_text().set(i, "World");
        }

        for (int i = 0; i < a.get_array_numbers().length; i++) {
            eStruct.get_array_numbers()[i] = a.get_array_numbers()[i] * b.get_array_numbers()[i];
            eStruct.get_array_text()[i] = "World";
        }

        for (int i = 0; i < a.get_binary_numbers().length; i++) {
            eStruct.get_binary_numbers()[i] = (byte) (a.get_binary_numbers()[i] * b.get_binary_numbers()[i]);
        }
    }

    @Override
    public boolean testSendingByrefMembers(StructWithByrefMembers s) {
        System.out.println("testSendingByrefMembers reached");
        return false;
    }
}
