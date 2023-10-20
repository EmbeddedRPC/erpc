package com.nxp.erpc.tests.server.services;

import com.nxp.erpc.tests.common.myEnum;
import com.nxp.erpc.tests.test_annotations.test.common.structs.fooStruct;
import com.nxp.erpc.tests.test_annotations.test.server.AbstractAnnotateTestService;


public class TestAnnotationsService extends AbstractAnnotateTestService {
    @Override
    public int add(int a, int b) {
        return a + b;
    }

    @Override
    public void testIfFooStructExist(fooStruct a) {
    }


    @Override
    public void testIfMyEnumExist(myEnum a) {

    }

    @Override
    public int testIfMyIntAndConstExist(int a) {
        return a;
    }
}
