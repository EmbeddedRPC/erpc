/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

package  io.github.embeddedrpc.erpc.auxiliary;

/**
 * Class for computing ITU-CCITT CRC-16.
 */
public class Crc16 {
    private static final int POLY = 0x1021;
    private final int crcStart;
    private final int[] table;

    /**
     * CRC16 constructor.
     *
     * @param crcStart CRC16 start position
     */
    @SuppressWarnings("magicnumber")
    public Crc16(int crcStart) {
        this.crcStart = crcStart;
        this.table = new int[256];
        for (int i = 0; i < 256; i++) {
            this.table[i] = this.computeTable(i);
        }
    }

    /**
     * CRC16 constructor with start position 0xEF4A.
     */
    @SuppressWarnings("magicnumber")
    public Crc16() {
        this(0xEF4A);
    }

    /**
     * Function return CRC16 for given byte array.
     *
     * @param data Input byte array
     * @return Computed CRC16
     */
    @SuppressWarnings("magicnumber")
    public int computeCRC16(byte[] data) {
        int crc = this.crcStart;
        for (byte c : data) {
            crc = ((crc << 8) ^ this.table[((crc >> 8) ^ c) & 0xFF]) & 0xFFFF;
        }
        return crc;
    }

    @SuppressWarnings("magicnumber")
    private int computeTable(int i) {
        int crc = 0;
        int y = i;
        y <<= 8;
        for (int b = 0; b < 8; b++) {
            int temp = crc ^ y;
            crc <<= 1;
            if ((temp & 0x8000) != 0) {
                crc ^= POLY;
            }
            y <<= 1;
        }
        return crc;
    }
}
