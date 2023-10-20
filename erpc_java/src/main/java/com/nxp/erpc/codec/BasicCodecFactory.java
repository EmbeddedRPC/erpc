/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

package com.nxp.erpc.codec;

/**
 * Implementation of CodeFactory for BasicCodec.
 */
public final class BasicCodecFactory extends CodecFactory {
    @Override
    protected Codec createCodec() {
        return new BasicCodec();
    }

    @Override
    protected Codec createCodec(final byte[] array) {
        return new BasicCodec(array);
    }

}
