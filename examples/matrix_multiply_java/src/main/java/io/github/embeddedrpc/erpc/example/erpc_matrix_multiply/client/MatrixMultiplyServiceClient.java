/** 
 * Generated by erpcgen 1.12.0 on Mon Jan  8 09:33:04 2024.
 * 
 * AUTOGENERATED - DO NOT EDIT
 */
 
package io.github.embeddedrpc.erpc.example.erpc_matrix_multiply.client;

import io.github.embeddedrpc.erpc.example.erpc_matrix_multiply.interfaces.IMatrixMultiplyService;




import io.github.embeddedrpc.erpc.auxiliary.MessageType;
import io.github.embeddedrpc.erpc.auxiliary.MessageInfo;
import io.github.embeddedrpc.erpc.auxiliary.RequestContext;
import io.github.embeddedrpc.erpc.auxiliary.Reference;
import io.github.embeddedrpc.erpc.auxiliary.Utils;
import io.github.embeddedrpc.erpc.client.ClientManager;
import io.github.embeddedrpc.erpc.codec.Codec;

import java.util.ArrayList;
import java.util.List;

public class MatrixMultiplyServiceClient implements IMatrixMultiplyService {
    private final ClientManager clientManager;

    public MatrixMultiplyServiceClient(ClientManager clientManager) {
        this.clientManager = clientManager;
    }

    @Override
    public void erpcMatrixMultiply(int[][] matrix1, int[][] matrix2, Reference<int[][]> result_matrix) {
        // Create request
        RequestContext request = clientManager.createRequest(false);
        Codec codec = request.codec();

        // Write message info
        codec.startWriteMessage(new MessageInfo(
                MessageType.kInvocationMessage,
                this.SERVICE_ID,
                this.ERPCMATRIXMULTIPLY_ID,
                request.sequence()));

        // Write message data
        Utils.checkNotNull(matrix1, "matrix1 is null");
        
        for (int genLoopVariable1 = 0; genLoopVariable1 < 5; ++genLoopVariable1)
        {
            int[] genValueVariable1 = matrix1[genLoopVariable1];
            for (int genLoopVariable2 = 0; genLoopVariable2 < 5; ++genLoopVariable2)
            {
                int genValueVariable2 = genValueVariable1[genLoopVariable2];
                codec.writeInt32(genValueVariable2);
            }
        }
        Utils.checkNotNull(matrix2, "matrix2 is null");
        
        for (int genLoopVariable1 = 0; genLoopVariable1 < 5; ++genLoopVariable1)
        {
            int[] genValueVariable1 = matrix2[genLoopVariable1];
            for (int genLoopVariable2 = 0; genLoopVariable2 < 5; ++genLoopVariable2)
            {
                int genValueVariable2 = genValueVariable1[genLoopVariable2];
                codec.writeInt32(genValueVariable2);
            }
        }

        // Perform request
        clientManager.performRequest(request);

        // Read out parameters
         
        result_matrix.set(new int[5][5]);
        for (int genLoopVariable1 = 0; genLoopVariable1 < 5; ++genLoopVariable1)
        {
            int[] genValueVariable1 = new int[5];
            for (int genLoopVariable2 = 0; genLoopVariable2 < 5; ++genLoopVariable2)
            {
                int genValueVariable2 = codec.readInt32();
                genValueVariable1[genLoopVariable2] = genValueVariable2;
            }
            result_matrix.get()[genLoopVariable1] = genValueVariable1;
        }
    }
}
