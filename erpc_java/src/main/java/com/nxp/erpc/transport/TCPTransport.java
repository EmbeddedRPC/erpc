/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

package com.nxp.erpc.transport;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.UnknownHostException;

/**
 * Implementation of the TCP transport.
 */
public final class TCPTransport extends FramedTransport {
    private final Socket socket;
    private final DataInputStream in;
    private final DataOutputStream out;

    /**
     * TCPTransport constructor.
     *
     * @param ip   IP of the eRPC server
     * @param port Port of the eRPC server
     * @throws UnknownHostException
     * @throws IOException
     */
    public TCPTransport(String ip, int port) throws UnknownHostException, IOException {
        socket = new Socket(ip, port);
        out = new DataOutputStream(socket.getOutputStream());
        in = new DataInputStream(socket.getInputStream());
    }

    /**
     * Create TCP transport.
     *
     * @param port TCP port
     * @throws IOException TCP error
     */
    public TCPTransport(int port) throws IOException {
        try (ServerSocket serverSocket = new ServerSocket(port)) {
            socket = serverSocket.accept();
        }
        out = new DataOutputStream(socket.getOutputStream());
        in = new DataInputStream(socket.getInputStream());
    }

    /**
     * Function closes all opened connections and streams.
     */
    @Override
    public void close() {
        try {
            in.close();
            out.close();
            socket.close();
        } catch (IOException e) {
            throw new TransportError("Error closing TCP connection", e);
        }
    }

    @Override
    public byte[] baseReceive(int count) {
        byte[] buffer = new byte[count];
        try {
            if (in.read(buffer, 0, count) != count) {
                throw new TransportError("Error receiving TCP data.");
            }
        } catch (IOException e) {
            throw new TransportError("Error receiving TCP.", e);
        }
        return buffer;
    }

    @Override
    public void baseSend(byte[] message) {
        try {
            out.write(message, 0, message.length);
            out.flush();
        } catch (IOException e) {
            throw new TransportError("Error sending TCP data", e);
        }

    }
}
