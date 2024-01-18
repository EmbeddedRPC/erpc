/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

package  io.github.embeddedrpc.erpc.transport;

/**
 * Transport interface.
 */
public interface Transport {
    /**
     * Send data through transport layer.
     *
     * @param message data to be sent.
     */
    void send(byte[] message);

    /**
     * Receive data from transport layer.
     *
     * @return byte array
     */
    byte[] receive();

    /**
     * Close transport layer.
     */
    void close();
}
