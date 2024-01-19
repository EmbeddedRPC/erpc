package io.github.embeddedrpc.erpc.example;

import io.github.embeddedrpc.erpc.auxiliary.Reference;
import io.github.embeddedrpc.erpc.example.Main;
import io.github.embeddedrpc.erpc.example.erpc_matrix_multiply.common.Constants;
import io.github.embeddedrpc.erpc.example.erpc_matrix_multiply.server.AbstractMatrixMultiplyServiceService;

public class MatrixMultiplyService extends AbstractMatrixMultiplyServiceService {

    @Override
    public void erpcMatrixMultiply(int[][] matrix1, int[][] matrix2, Reference<int[][]> result_matrix) {
        System.out.println("Server received these matrices:");
        System.out.println("\r\nMatrix #1");
        System.out.println("=========");
        Main.printMatrix(matrix1);

        System.out.println("\r\nMatrix #2");
        System.out.println("=========");
        Main.printMatrix(matrix2);

        result_matrix.set(new int[Constants.matrix_size][Constants.matrix_size]);

        for (int i = 0; i < Constants.matrix_size; i++) {
            for (int j = 0; j < Constants.matrix_size; j++) {
                for (int k = 0; k < Constants.matrix_size; k++) {
                    result_matrix.get()[i][j] += matrix1[i][k] * matrix2[k][j];
                }
            }
        }

        System.out.println("\r\nResult matrix");
        System.out.println("=========");
        Main.printMatrix(result_matrix.get());
    }
}
