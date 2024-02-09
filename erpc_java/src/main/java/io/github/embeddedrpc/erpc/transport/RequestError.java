/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

package  io.github.embeddedrpc.erpc.transport;

/**
 * Request error.
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
}
