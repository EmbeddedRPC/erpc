package io.github.embeddedrpc.erpc.tests.client;

import io.github.embeddedrpc.erpc.tests.test_annotations.test.client.AnnotateTestClient;
import io.github.embeddedrpc.erpc.tests.TestingClient;
import io.github.embeddedrpc.erpc.tests.test_annotations.test.interfaces.IAnnotateTest;
import io.github.embeddedrpc.erpc.tests.test_arrays.test_unit_test_common.client.CommonClient;
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
