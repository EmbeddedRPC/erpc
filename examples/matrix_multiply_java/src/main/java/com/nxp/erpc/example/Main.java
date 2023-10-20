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
import com.nxp.erpc.transport.TCPTransport;
import com.nxp.erpc.transport.Transport;

import java.io.IOException;
import java.util.Random;

public class Main {
    public static void main(String[] args) throws IOException {
        if (args.length == 0) {
            System.err.println("Select server or client.");
            System.exit(-1);
        }

        if (args[0].equals("client")) {
            client();
        } else if (args[0].equals("server")) {
            server();
        } else {
            System.err.println("Unknown argument.");
            System.exit(-1);
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

    public static void server() throws IOException {
        TCPTransport transport = new TCPTransport(40);
        Server server = new SimpleServer(transport, new BasicCodecFactory());

        server.addService(new MatrixMultiplyService());

        server.run();
    }

    public static void client() throws IOException {
        /* Uncomment required transport */
        Transport transport = new TCPTransport("localhost", 40);
        // SerialTransport transport = new SerialTransport("COM4", 115200);

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

                System.out.println("\r\nPress Enter to initiate the next matrix multiplication");
                System.in.read();
            }
        } finally {
            transport.close();
        }
    }
}