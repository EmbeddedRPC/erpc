/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

package com.nxp.erpc.auxiliary;

/**
 * Request error thrown when unexpected situation occurred during sending and receiving message.
 */
public class RequestError extends RuntimeException {
    /**
     * Request error.
     *
     * @param message error message
     */
    public RequestError(String message) {
        super(message);
    }

    /**
     * Request error.
     * @param message error message
     * @param cause error chain
     */
    public RequestError(String message, Throwable cause) {
        super(message, cause);
    }
}
