/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

package com.nxp.erpc.codec;

/**
 * Codec error.
 */
public class CodecError extends RuntimeException {
    /**
     * Request error.
     *
     * @param message error message
     */
    public CodecError(String message) {
        super(message);
    }
}
