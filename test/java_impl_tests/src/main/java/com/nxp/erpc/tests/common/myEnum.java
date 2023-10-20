package com.nxp.erpc.tests.common;

import com.nxp.erpc.codec.Codec;

/**
 * Custom enum implementation for testing.
 */
public enum myEnum {
    one(0),
    two(1),
    three(2);

    private final Integer value;

    myEnum(Integer value) {
        this.value = value;
    }

    public int getValue() {
        return value;
    }

    public static myEnum get(int value) {
        if (value == 0) {
            return one;
        }

        if (value == 1) {
            return two;
        }

        if (value == 2) {
            return three;
        }

        return null;
    }

    public static myEnum read(Codec codec) {
        return get(codec.readInt32());
    }

    public void write(Codec codec) {
        codec.writeInt32(this.getValue());
    }

}
