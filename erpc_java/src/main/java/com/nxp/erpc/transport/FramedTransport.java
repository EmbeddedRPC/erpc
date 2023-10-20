/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

package com.nxp.erpc.transport;

import com.nxp.erpc.auxiliary.Crc16;
import com.nxp.erpc.codec.BasicCodec;
import com.nxp.erpc.codec.Codec;

import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * Abstract class for framed transports implementations.
 */
public abstract class FramedTransport implements Transport {
    private static final int HEADER_LEN = 4;

    private final Crc16 crc16 = new Crc16();
    private final Lock receiveLock = new ReentrantLock();
    private final Lock sendLock = new ReentrantLock();

    @Override
    public final byte[] receive() {
        try {
            receiveLock.lock();
            byte[] headerData = baseReceive(HEADER_LEN);
            Codec codec = new BasicCodec(headerData);

            int messageLength = codec.readUInt16();
            int receivedCrc16 = codec.readUInt16();

            byte[] data = baseReceive(messageLength);

            int computedCrc16 = crc16.computeCRC16(data);

            if (computedCrc16 != receivedCrc16) {
                throw new RequestError("Invalid message CRC");
            }

            return data;
        } finally {
            receiveLock.unlock();
        }

    }

    @Override
    public final void send(byte[] message) {
        try {
            sendLock.lock();

            int computedCrc16 = crc16.computeCRC16(message);

            Codec codec = new BasicCodec();
            codec.writeUInt16(message.length);
            codec.writeUInt16(computedCrc16);

            byte[] header = codec.array();

            assert header.length == HEADER_LEN;

            byte[] data = new byte[message.length + header.length];

            System.arraycopy(header, 0, data, 0, header.length);
            System.arraycopy(message, 0, data, header.length, message.length);

            baseSend(data);
        } finally {
            sendLock.unlock();
        }

    }

    abstract void baseSend(byte[] data);

    abstract byte[] baseReceive(int count);
}
