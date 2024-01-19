package io.github.embeddedrpc.erpc.tests;

import io.github.embeddedrpc.erpc.transport.SerialTransport;
import io.github.embeddedrpc.erpc.transport.TCPTransport;
import io.github.embeddedrpc.erpc.transport.Transport;

import java.io.IOException;

public final class Connection {
    public static final String HOST = "localhost";
    public static final int PORT = 40;
    public static final int BAUDRATE = 115200;

    private Connection() {
    }

    /**
     * Create transport based on properties:
     * -Dserver or -Dclient (default: client) - Server or client
     * -Dhost=HOST_NAME (default: localhost) - Host name
     * -Dport=PORT (default: 40) - Port
     * -Dserial=COM_PORT (default: null) - Serial communication
     * -Dbaudrate=BAUDRATE (default: 115200) - Serial link baudrate
     *
     * @return New transport
     */
    public static Transport getConnectionFromSystemProperties() {
        String host = HOST;
        int port = PORT;
        String serial = null;
        int baudrate = BAUDRATE;
        boolean isServer = System.getProperty("server") != null;

        // Read parameters
        if (System.getProperty("host") != null && !System.getProperty("host").isEmpty()) {
            host = System.getProperty("host");
        }

        if (System.getProperty("port") != null && !System.getProperty("port").isEmpty()) {
            port = Integer.parseInt(System.getProperty("port"));
        }

        if (System.getProperty("serial") != null) {
            if (System.getProperty("serial").isEmpty()) {
                System.err.println("Select serial port with: '-Dserial=COM_PORT'");
                System.exit(-1);
            }
            serial = System.getProperty("serial");
        }

        if (System.getProperty("baudrate") != null && !System.getProperty("baudrate").isEmpty()) {
            baudrate = Integer.parseInt(System.getProperty("baudrate"));
        }

        if (serial != null) {
            return new SerialTransport(serial, baudrate);
        } else {
            try {
                if (isServer) {
                    return new TCPTransport(port);
                } else {
                    // client
                    return new TCPTransport(host, port);
                }
            } catch (IOException e) {
                throw new RuntimeException("Error while creating TCPTransport(" + host + ", " + port + ")", e);
            }
        }
    }
}
