/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

package  io.github.embeddedrpc.erpc.transport;

import  io.github.embeddedrpc.erpc.auxiliary.Crc16;
import  io.github.embeddedrpc.erpc.auxiliary.Utils;
import  io.github.embeddedrpc.erpc.codec.BasicCodec;
import  io.github.embeddedrpc.erpc.codec.Codec;

import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * Abstract class for framed transports implementations.
 */
public abstract class FramedTransport implements Transport {
    private static final int HEADER_LEN = 6;

    private final Crc16 crc16 = new Crc16();
    private final Lock receiveLock = new ReentrantLock();
    private final Lock sendLock = new ReentrantLock();

    @Override
    public final byte[] receive() {
        try {
            receiveLock.lock();
            byte[] headerData = baseReceive(HEADER_LEN);
            Codec codec = new BasicCodec(headerData);

            int crcHeader = codec.readUInt16();
            int messageLength = codec.readUInt16();
            int crcBody = codec.readUInt16();

            int computedCrc = crc16.computeCRC16(Utils.uInt16ToBytes(messageLength))
                    + crc16.computeCRC16(Utils.uInt16ToBytes(crcBody));
            computedCrc &= 0xFFFF; // 2 bytes

            if (computedCrc != crcHeader) {
                throw new RequestError("Invalid message (header) CRC");
            }

            byte[] data = baseReceive(messageLength);

            int computedBodyCrc16 = crc16.computeCRC16(data);

            if (computedBodyCrc16 != crcBody) {
                throw new RequestError("Invalid message (body) CRC");
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

            Codec codec = new BasicCodec();

            int messageLength = message.length;
            int crcBody = crc16.computeCRC16(message);
            int crcHeader = crc16.computeCRC16(Utils.uInt16ToBytes(messageLength))
                    + crc16.computeCRC16(Utils.uInt16ToBytes(crcBody));
            crcHeader &= 0xFFFF; // 2 bytes

            codec.writeUInt16(crcHeader);
            codec.writeUInt16(messageLength);
            codec.writeUInt16(crcBody);

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
