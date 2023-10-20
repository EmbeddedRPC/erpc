package com.nxp.erpc.tests.client;

import com.nxp.erpc.tests.test_annotations.test.client.AnnotateTestClient;
import com.nxp.erpc.tests.TestingClient;
import com.nxp.erpc.tests.test_annotations.test.interfaces.IAnnotateTest;
import com.nxp.erpc.tests.test_arrays.test_unit_test_common.client.CommonClient;
import org.junit.jupiter.api.AfterAll;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.TestInstance;

import static org.junit.jupiter.api.Assertions.*;

@TestInstance(TestInstance.Lifecycle.PER_CLASS)
public class TestAnnotations extends TestingClient {
    private final AnnotateTestClient client = new AnnotateTestClient(clientManager);

    @Test
    public void runTestAnnotationServiceID() {
        assertEquals(5, IAnnotateTest.SERVICE_ID);
    }

    @Test
    public void runTesttestIfMyIntAndConstExist() {
        assertEquals(1, client.testIfMyIntAndConstExist(1));
    }

    @AfterAll
    public void quit() {
        new CommonClient(clientManager).quit();
    }
}
