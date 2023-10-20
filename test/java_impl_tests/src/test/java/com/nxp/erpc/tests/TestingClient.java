/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

package com.nxp.erpc.tests;

import com.nxp.erpc.client.ClientManager;
import com.nxp.erpc.codec.BasicCodecFactory;
import com.nxp.erpc.codec.Codec;
import com.nxp.erpc.tests.Connection;
import com.nxp.erpc.transport.SerialTransport;
import com.nxp.erpc.transport.TCPTransport;
import com.nxp.erpc.transport.Transport;

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
