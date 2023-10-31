/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

package com.nxp.erpc.example;

import com.nxp.erpc.auxiliary.Reference;
import com.nxp.erpc.client.ClientManager;
import com.nxp.erpc.codec.BasicCodecFactory;
import com.nxp.erpc.example.erpc_matrix_multiply.client.MatrixMultiplyServiceClient;
import com.nxp.erpc.example.erpc_matrix_multiply.common.Constants;
import com.nxp.erpc.server.Server;
import com.nxp.erpc.server.SimpleServer;
import com.nxp.erpc.transport.SerialTransport;
import com.nxp.erpc.transport.TCPTransport;
import com.nxp.erpc.transport.Transport;
import org.apache.commons.cli.*;

import java.io.IOException;
import java.util.Random;

public class Main {
    public static final String HOST = "localhost";
    public static final int PORT = 40;
    public static final int BAUDRATE = 115200;

    public static void main(String[] args) throws IOException {
        // parsing objects
        Options options = new Options();
        HelpFormatter formatter = new HelpFormatter();
        CommandLine cmd;

        // configuration
        String host = HOST;
        int port = PORT;
        String serial = null;
        int baudrate = BAUDRATE;
        Transport transport;

        // parsing options
        options.addOption("c", "client", false, "Run client");
        options.addOption("s", "server", false, "Run server");
        options.addOption("t", "host", true, "Host IP address (default value is localhost)");
        options.addOption("p", "port", true, "Port (default value is 40)");
        options.addOption("S", "serial", true, "Serial device (default value is None)");
        options.addOption("B", "baud", true, "Baud (default value is 115200)");

        CommandLineParser parser = new DefaultParser();

        // parsing cmd arguments
        try {
            cmd = parser.parse(options, args);
        } catch (ParseException e) {
            System.err.println("Parsing failed.  Reason: " + e.getMessage());
            System.exit(1);
            return;
        }

        if (!cmd.hasOption('c') && !cmd.hasOption('s')) {
            formatter.printHelp("ant", options);
            System.err.println("Either server or client has to be selected to run");
            System.exit(1);
        }

        if (cmd.hasOption('t') && !cmd.getOptionValue('t').isEmpty()) {
            host = cmd.getOptionValue('t');
        }

        if (cmd.hasOption('p') && !cmd.getOptionValue('p').isEmpty()) {
            port = Integer.parseInt(cmd.getOptionValue('p'));
        }

        if (cmd.hasOption("S")) {
            if (cmd.getOptionValue('S').isEmpty()) {
                System.err.println("Select serial port with: '-S COM_PORT'");
                System.exit(1);
            }
            serial = cmd.getOptionValue('S');
        }

        if (cmd.hasOption('B') && !cmd.getOptionValue('B').isEmpty()) {
            baudrate = Integer.parseInt(System.getProperty("baudrate"));
        }

        System.out.println("eRPC Matrix Multiply TCP example");

        // transport initialization
        if (serial != null) {
            // initialize Serial transport layer
            System.out.println("Transport: serial=" + serial + " baudrate=" + baudrate);
            transport = new SerialTransport(serial, baudrate);
        } else {
            // initialize TCP transport layer
            if (cmd.hasOption('c')) {
                // client
                System.out.println("Transport: tcp client - host=" + host + " port=" + port);
                transport = new TCPTransport(host, port);
            } else {
                // server
                System.out.println("Transport: tcp server - port = " + port);
                transport = new TCPTransport(port);
            }
        }

        // running example
        if (cmd.hasOption('c')) {
            client(transport);
        } else if (cmd.hasOption('s')) {
            server(transport);
        } else {
            // Should not happen
            assert false;
        }
    }

    public static void fillMatrix(int[][] matrix) {
        Random rand = new Random();
        for (int y = 0; y < matrix.length; y++) {
            for (int x = 0; x < matrix[y].length; x++) {
                matrix[y][x] = Math.abs(rand.nextInt() % 10);
            }
        }
    }

    public static void printMatrix(int[][] matrix) {
        for (int[] y : matrix) {
            for (int x : y) {
                System.out.print(x);
                System.out.print(" ");
            }
            System.out.println();
        }
    }

    public static void server(Transport transport) {
        Server server = new SimpleServer(transport, new BasicCodecFactory());

        server.addService(new MatrixMultiplyService());

        server.run();
    }

    public static void client(Transport transport) throws IOException {
        ClientManager clientManager = new ClientManager(transport, new BasicCodecFactory());
        MatrixMultiplyServiceClient client = new MatrixMultiplyServiceClient(clientManager);

        try {
            while (true) {
                int[][] matrix1 = new int[Constants.matrix_size][5];
                int[][] matrix2 = new int[5][5];
                Reference<int[][]> matrixResult = new Reference<>();

                fillMatrix(matrix1);
                fillMatrix(matrix2);

                System.out.println("Matrix #1\r\n=========");
                printMatrix(matrix1);
                System.out.println("\r\nMatrix #1\r\n=========");
                printMatrix(matrix2);

                System.out.println("\r\neRPC request is sent to the server");
                client.erpcMatrixMultiply(matrix1, matrix2, matrixResult);

                System.out.println("\r\nMatrix result\r\n=========");
                printMatrix(matrixResult.get());

                System.out.println("\r\nPress Enter to initiate the next matrix multiplication or 'q' to quit");
                if (System.in.read() == 'q') {
                    break;
                }
            }
        } finally {
            transport.close();
        }

        System.out.println("eRPC Matrix Multiply TCP example finished.");
    }
}