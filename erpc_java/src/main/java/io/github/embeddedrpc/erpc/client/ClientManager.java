/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

package  io.github.embeddedrpc.erpc.client;

import  io.github.embeddedrpc.erpc.codec.Codec;
import  io.github.embeddedrpc.erpc.codec.CodecFactory;
import  io.github.embeddedrpc.erpc.transport.Transport;
import  io.github.embeddedrpc.erpc.auxiliary.MessageType;
import  io.github.embeddedrpc.erpc.auxiliary.MessageInfo;
import  io.github.embeddedrpc.erpc.auxiliary.RequestContext;
import  io.github.embeddedrpc.erpc.auxiliary.RequestError;

/**
 * Client manager.
 */
public class ClientManager {
    private final Transport transport;
    private final CodecFactory codecFactory;
    private int sequence;

    /**
     * ClientManager constructor.
     *
     * @param transport    Transport used for communication
     * @param codecFactory Codec factory used for creating codec instances
     */
    public ClientManager(final Transport transport,
                         final CodecFactory codecFactory) {
        this.transport = transport;
        this.codecFactory = codecFactory;
        this.sequence = 0;
    }

    private int getSequence() {
        sequence += 1;
        return sequence;
    }

    /**
     * Function creates new request context.
     *
     * @param isOneWay Specify request direction
     * @return New request context
     */
    public RequestContext createRequest(Boolean isOneWay) {
        Codec codec = codecFactory.create();

        return new RequestContext(getSequence(), null, codec, isOneWay);
    }

    /**
     * Function takes @{RequestContext} and perform data exchange with server.
     * If request is not one way, read incoming data and save them to request's
     * codec.
     *
     * @param request Request to be performed
     */
    public void performRequest(RequestContext request) {
        this.transport.send(request.codec().array());

        if (!request.isOneWay()) {
            request.codec().setArray(this.transport.receive());

            MessageInfo info = request.codec().startReadMessage();

            if (info.type() != MessageType.kReplyMessage) {
                throw new RequestError("Invalid reply message type.");
            }

            if (info.sequence() != request.sequence()) {
                throw new RequestError("Unexpected sequence number in reply (was " + info.sequence() + ", expected "
                        + request.sequence() + ").");
            }
        }
    }

}
