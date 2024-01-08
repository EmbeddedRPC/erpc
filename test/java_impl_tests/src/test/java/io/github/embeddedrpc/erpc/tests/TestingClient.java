/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

package io.github.embeddedrpc.erpc.tests;

import io.github.embeddedrpc.erpc.client.ClientManager;
import io.github.embeddedrpc.erpc.codec.BasicCodecFactory;
import io.github.embeddedrpc.erpc.codec.Codec;
import io.github.embeddedrpc.erpc.tests.Connection;
import io.github.embeddedrpc.erpc.transport.SerialTransport;
import io.github.embeddedrpc.erpc.transport.TCPTransport;
import io.github.embeddedrpc.erpc.transport.Transport;

import java.io.IOException;

public abstract class TestingClient {
    protected final ClientManager clientManager;

    public TestingClient() {
        clientManager = new ClientManager(
                Connection.getConnectionFromSystemProperties(),
                new BasicCodecFactory()
        );
    }
}
