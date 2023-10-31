/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

package com.nxp.erpc.transport;

import com.fazecast.jSerialComm.SerialPort;

import static com.fazecast.jSerialComm.SerialPort.TIMEOUT_READ_BLOCKING;
import static com.fazecast.jSerialComm.SerialPort.TIMEOUT_WRITE_BLOCKING;

/**
 * Serial transport implementation.
 */
public final class SerialTransport extends FramedTransport {
    private static final int READ_TIMEOUT = 500;
    private static final int WRITE_TIMEOUT = 500;
    private final SerialPort serial;

    /**
     * Serial transport constructor.
     *
     * @param url      Serial port URL
     * @param baudrate Communication baud rate
     */
    public SerialTransport(String url, int baudrate) {
        super();

        serial = SerialPort.getCommPort(url);
        serial.setBaudRate(baudrate);
        serial.setComPortTimeouts(
                TIMEOUT_READ_BLOCKING | TIMEOUT_WRITE_BLOCKING,
                READ_TIMEOUT,
                WRITE_TIMEOUT
        );

        if (!serial.openPort()) {
            throw new TransportError("Unable to open port \"" + url + "\"");
        }
    }

    @Override
    public void baseSend(byte[] message) {
        assert serial.isOpen();

        int bytesSent = serial.writeBytes(message, message.length);

        if (bytesSent != message.length) {
            throw new TransportError("Should send: " + message.length + ", but sent: " + bytesSent);
        }
    }

    @Override
    public byte[] baseReceive(int count) {
        assert serial.isOpen();

        byte[] received = new byte[count];
        int bytesRead = serial.readBytes(received, count);

        if (bytesRead != count) {
            throw new TransportError("Should read: " + count + ", but read: " + bytesRead);
        }

        return received;
    }

    @Override
    public void close() {
        if (serial.isOpen()) {
            serial.closePort();
        }
    }

}
