/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

package  io.github.embeddedrpc.erpc.codec;

import  io.github.embeddedrpc.erpc.auxiliary.MessageInfo;
import  io.github.embeddedrpc.erpc.auxiliary.MessageType;
import  io.github.embeddedrpc.erpc.auxiliary.Utils;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * Basic implementation of the Codec.
 */
@SuppressWarnings("checkstyle:MagicNumber")
public final class BasicCodec implements Codec {
    private static final int BASIC_CODEC_VERSION = 1;
    private static final int DEFAULT_BUFFER_SIZE = 256;

    private ByteBuffer buffer;

    /**
     * Basic codec constructor. Create empty buffer.
     */
    public BasicCodec() {
        this.reset();
    }

    /**
     * Basic codec constructor. Create buffer from byte array.
     *
     * @param byteArray byte array
     */
    public BasicCodec(final byte[] byteArray) {
        this.buffer = ByteBuffer.wrap(byteArray).order(ByteOrder.LITTLE_ENDIAN);
    }

    private void prepareForWrite(final int bytesToWrite) {
        if (this.buffer.remaining() < bytesToWrite) {
            ByteBuffer newBuffer = ByteBuffer.allocate(
                    (this.buffer.capacity() + bytesToWrite) * 2
            );
            newBuffer.order(ByteOrder.LITTLE_ENDIAN);
            this.buffer.flip();

            newBuffer.put(this.buffer);
            this.buffer = newBuffer;
        }
    }

    @Override
    public void reset() {
        this.buffer = ByteBuffer.allocate(DEFAULT_BUFFER_SIZE)
                .order(ByteOrder.LITTLE_ENDIAN);
    }

    @Override
    public byte[] array() {
        int position = buffer.position();
        byte[] data = new byte[position];
        buffer.position(0);
        buffer.get(data);
        buffer.position(position);
        return data;
    }

    @Override
    public void setArray(final byte[] array) {
        this.buffer = ByteBuffer.wrap(array).order(ByteOrder.LITTLE_ENDIAN);
    }

    @Override
    public void startWriteMessage(final MessageInfo msgInfo) {
        long header = (BASIC_CODEC_VERSION << 24)
                | ((msgInfo.service() & 0xff) << 16)
                | ((msgInfo.request() & 0xff) << 8)
                | (msgInfo.type().getValue() & 0xff);

        this.writeUInt32(header);
        this.writeUInt32(msgInfo.sequence());
    }

    @Override
    public void writeBool(final Boolean value) {
        prepareForWrite(1);
        this.buffer.put(value ? (byte) 1 : (byte) 0);
    }

    @Override
    public void writeInt8(final byte value) {
        prepareForWrite(1);
        this.buffer.put(value);
    }

    @Override
    public void writeInt16(final short value) {
        prepareForWrite(2);
        this.buffer.putShort(value);
    }

    @Override
    public void writeInt32(final int value) {
        prepareForWrite(4);
        this.buffer.putInt(value);
    }

    @Override
    public void writeInt64(final long value) {
        prepareForWrite(8);
        this.buffer.putLong(value);
    }

    @Override
    public void writeUInt8(final short value) {
        Utils.checkUInt8(value);
        prepareForWrite(1);
        this.buffer.put(Utils.uInt8toByte(value));
    }

    @Override
    public void writeUInt16(final int value) {
        Utils.checkUInt16(value);
        prepareForWrite(2);
        this.buffer.putShort(Utils.uInt16toShort(value));
    }

    @Override
    public void writeUInt32(final long value) {
        Utils.checkUInt32(value);
        prepareForWrite(4);
        this.buffer.putInt(Utils.uInt32toInt(value));
    }

    @Override
    public void writeUInt64(final long value) {
        throw new UnsupportedOperationException(
                "Java implementation of the eRPC does not support 'uint64'"
        );
    }

    @Override
    public void writeFloat(final float value) {
        prepareForWrite(4);
        this.buffer.putFloat(value);
    }

    @Override
    public void writeDouble(final double value) {
        prepareForWrite(8);
        this.buffer.putDouble(value);
    }

    @Override
    public void writeString(final String value) {
        this.writeBinary(value.getBytes());
    }

    @Override
    public void writeBinary(final byte[] value) {
        this.writeInt32(value.length);
        prepareForWrite(value.length);
        this.buffer.put(value);
    }

    @Override
    public void startWriteList(final int length) {
        this.writeUInt32(length);
    }

    @Override
    public void startWriteUnion(final int discriminator) {
        this.writeUInt32(discriminator);
    }

    @Override
    public void writeNullFlag(final int value) {
        this.writeUInt32(value != 0 ? 1 : 0);
    }

    @Override
    public MessageInfo startReadMessage() {
        int header = (int) this.readUInt32();
        int sequence = (int) this.readUInt32();
        int version = header >> 24;

        if (version != BASIC_CODEC_VERSION) {
            throw new CodecError("Unsupported codec version" + version);
        }

        int service = (header >> 16) & 0xff;
        int request = (header >> 8) & 0xff;
        MessageType msgType = MessageType.getMessageType(header & 0xff);

        return new MessageInfo(msgType, service, request, sequence);
    }

    @Override
    public Boolean readBool() {
        return this.buffer.get() != 0;
    }

    @Override
    public byte readInt8() {
        return this.buffer.get();
    }

    @Override
    public short readInt16() {
        return this.buffer.getShort();
    }

    @Override
    public int readInt32() {
        return this.buffer.getInt();
    }

    @Override
    public long readInt64() {
        return this.buffer.getLong();
    }

    @Override
    public short readUInt8() {
        return Utils.byteToUInt8(this.buffer.get());
    }

    @Override
    public int readUInt16() {
        return Utils.shortToUInt16(this.buffer.getShort());
    }

    @Override
    public long readUInt32() {
        return Utils.intToUInt32(this.buffer.getInt());
    }

    @Override
    public long readUInt64() {
        throw new UnsupportedOperationException(
                "Java implementation of the eRPC does not support 'uint64'"
        );
    }

    @Override
    public float readFloat() {
        return this.buffer.getFloat();
    }

    @Override
    public double readDouble() {
        return this.buffer.getDouble();
    }

    @Override
    public String readString() {
        return new String(this.readBinary());
    }

    @Override
    public byte[] readBinary() {
        long length = readUInt32();
        byte[] data = new byte[(int) length];
        this.buffer.get(data, 0, (int) length);

        return data;
    }

    @Override
    public long startReadList() {
        return this.readUInt32();
    }

    @Override
    public int startReadUnion() {
        return this.readInt8();
    }

    @Override
    public Boolean readNullFlag() {
        return this.readUInt8() != 0;
    }

    @Override
    public ByteBuffer getBuffer() {
        return buffer;
    }
}
