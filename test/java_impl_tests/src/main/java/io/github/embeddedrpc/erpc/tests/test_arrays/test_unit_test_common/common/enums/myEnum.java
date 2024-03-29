/*!
 * Copyright (c) 2014-2015, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/** 
 * Generated by erpcgen 1.12.0 on Mon Jan  8 10:25:43 2024.
 * 
 * AUTOGENERATED - DO NOT EDIT
 */

package io.github.embeddedrpc.erpc.tests.test_arrays.test_unit_test_common.common.enums;

import io.github.embeddedrpc.erpc.codec.Codec;

public enum myEnum{
    a(0), 
    b(1), 
    c(2); 

    private final Integer value;

    myEnum(Integer value) {
        this.value = value;
    }

    public int getValue() {
        return value;
    }

    public static myEnum get(int value) {
        if (value == 0) {
            return a;
        }
        
        if (value == 1) {
            return b;
        }
        
        if (value == 2) {
            return c;
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