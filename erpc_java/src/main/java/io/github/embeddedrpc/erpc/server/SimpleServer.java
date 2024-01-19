/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

package  io.github.embeddedrpc.erpc.server;

import  io.github.embeddedrpc.erpc.codec.Codec;
import  io.github.embeddedrpc.erpc.codec.CodecFactory;
import  io.github.embeddedrpc.erpc.transport.RequestError;
import  io.github.embeddedrpc.erpc.transport.Transport;
import  io.github.embeddedrpc.erpc.transport.TransportError;

/**
 * Simple server implementation. Single thread.
 */
public final class SimpleServer extends Server {
    private boolean runFlag;

    /**
     * Crea simple server.
     *
     * @param transport    transport used in server
     * @param codecFactory codec factory used for creating codec on request
     */
    public SimpleServer(Transport transport, CodecFactory codecFactory) {
        super(transport, codecFactory);
    }

    private void receiveRequest() {
        byte[] data = getTransport().receive();
        Codec codec = getCodecFactory().create(data);

        processRequest(codec);

        byte[] responseData = codec.array();

        if (responseData.length != 0) {
            getTransport().send(responseData);
        }
    }

    @Override
    public void run() {
        runFlag = true;

        while (runFlag) {
            try {
                receiveRequest();
            } catch (TransportError e) {
                System.err.println("Transport error: " + e);
                stop();
            } catch (RequestError e) {
                System.err.println("Error while processing request: " + e);
                stop();
            }
        }
    }

    @Override
    public void stop() {
        runFlag = false;
    }

}
