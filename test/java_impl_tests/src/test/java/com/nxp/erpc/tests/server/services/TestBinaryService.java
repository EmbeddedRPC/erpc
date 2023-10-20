package com.nxp.erpc.tests.server.services;

import com.nxp.erpc.auxiliary.Reference;
import com.nxp.erpc.tests.test_binary.erpc_outputs.test.server.AbstractBinaryService;

public class TestBinaryService extends AbstractBinaryService {
    @Override
    public void sendBinary(byte[] a) {
        System.out.println("sendBinary reached");
    }

    @Override
    public void test_binary_allDirection(byte[] a, byte[] b, Reference<byte[]> e) {
        System.out.println("test_binary_allDirection reached");
        e.set(new byte[5]);

        for (int i = 0; i < 5; i++) {
            e.get()[i] = (byte) (a[i] * b[i]);
        }
    }

    @Override
    public void test_binary_allDirectionLength(byte[] a, byte[] b, Reference<byte[]> d) {
        System.out.println("test_binary_allDirectionLength reached");
    }
}
