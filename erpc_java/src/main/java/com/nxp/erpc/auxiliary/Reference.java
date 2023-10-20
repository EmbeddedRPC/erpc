/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

package com.nxp.erpc.auxiliary;

/**
 * This class is used for out/inout parameters.
 *
 * @param <T> Type of value stored in Reference
 */
public final class Reference<T> {
    private T referent;

    /**
     * Reference constructor with initial value.
     *
     * @param initialValue default value
     */
    public Reference(T initialValue) {
        referent = initialValue;
    }

    /**
     * Reference constructor.
     */
    public Reference() {
    }

    /**
     * Set reference value.
     *
     * @param newVal new value
     */
    public void set(T newVal) {
        referent = newVal;
    }

    /**
     * Get reference value.
     *
     * @return value
     */
    public T get() {
        return referent;
    }
}
