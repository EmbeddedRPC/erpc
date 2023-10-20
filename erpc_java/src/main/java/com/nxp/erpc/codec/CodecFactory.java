/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

package com.nxp.erpc.codec;

/**
 * Abstract class for Code factory. Allow to create specific codec.
 */
public abstract class CodecFactory {
    /**
     * Create new empty Codec.
     *
     * @return Empty Codec
     */
    public Codec create() {
        return createCodec();
    }

    /**
     * Create new Codec from initial bytes.
     *
     * @param array Initial bytes for Codec
     * @return New Codec
     */
    public Codec create(byte[] array) {
        return createCodec(array);
    }

    protected abstract Codec createCodec();

    protected abstract Codec createCodec(byte[] array);
}
