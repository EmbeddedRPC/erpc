/** 
 * Generated by erpcgen 1.12.0 on Mon Jan  8 10:25:43 2024.
 * 
 * AUTOGENERATED - DO NOT EDIT
 */
package io.github.embeddedrpc.erpc.tests.test_arrays.test_unit_test_common.server;

import io.github.embeddedrpc.erpc.tests.test_arrays.test_unit_test_common.interfaces.ICommon;

import io.github.embeddedrpc.erpc.tests.test_arrays.test_unit_test_common.common.enums.*;

import io.github.embeddedrpc.erpc.auxiliary.MessageInfo;
import io.github.embeddedrpc.erpc.auxiliary.MessageType;
import io.github.embeddedrpc.erpc.codec.Codec;
import io.github.embeddedrpc.erpc.server.Service;
import io.github.embeddedrpc.erpc.auxiliary.Reference;


import java.util.ArrayList;
import java.util.List;

/**
 * Testing abstract service class for simple eRPC interface.
 */
public abstract class AbstractCommonService
        extends Service
        implements ICommon {

    /**
     * Default constructor.
     */
    public AbstractCommonService() {
        super(ICommon.SERVICE_ID);
        addMethodHandler(ICommon.QUIT_ID, this::quitHandler);
        addMethodHandler(ICommon.GETSERVERALLOCATED_ID, this::getServerAllocatedHandler);
    }

    private void quitHandler(int sequence, Codec codec) {


        quit();

        codec.reset();

    }
    private void getServerAllocatedHandler(int sequence, Codec codec) {


        int _result = getServerAllocated();

        codec.reset();

        codec.startWriteMessage(new MessageInfo(
                MessageType.kReplyMessage,
                getServiceId(),
                ICommon.GETSERVERALLOCATED_ID,
                sequence)
        );

        // Read out parameters
        // Read return value
        codec.writeInt32(_result);
    }

}

