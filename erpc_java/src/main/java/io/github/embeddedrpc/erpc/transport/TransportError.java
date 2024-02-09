/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

package  io.github.embeddedrpc.erpc.transport;

/**
 * Transport runtime exception.
 */
public class TransportError extends RuntimeException {
    /**
     * Transport error.
     */
    public TransportError() {
        super();
    }

    /**
     * Transport error.
     *
     * @param message error message
     */
    public TransportError(String message) {
        super(message);
    }

    /**
     * Transport error.
     *
     * @param message error message
     * @param cause   error chain
     */
    public TransportError(String message, Throwable cause) {
        super(message, cause);
    }
}
