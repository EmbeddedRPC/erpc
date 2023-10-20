/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

package com.nxp.erpc.server;

import com.nxp.erpc.auxiliary.MessageInfo;
import com.nxp.erpc.auxiliary.MessageType;
import com.nxp.erpc.auxiliary.RequestError;
import com.nxp.erpc.codec.Codec;
import com.nxp.erpc.codec.CodecFactory;
import com.nxp.erpc.transport.Transport;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

/**
 * Abstract server class.
 */
public abstract class Server {
    private final CodecFactory codecFactory;
    private final Transport transport;
    private final Map<Integer, Service> services = new HashMap<>();

    /**
     * Server constructor.
     *
     * @param transport    transport used in server
     * @param codecFactory codec factory used for creating codec
     */
    public Server(Transport transport, CodecFactory codecFactory) {
        this.transport = transport;
        this.codecFactory = codecFactory;
    }

    protected final CodecFactory getCodecFactory() {
        return codecFactory;
    }

    protected final Transport getTransport() {
        return transport;
    }

    /**
     * Get unmodifiable map of all services.
     *
     * @return map of services.
     */
    public Map<Integer, Service> getServices() {
        return Collections.unmodifiableMap(services);
    }

    /**
     * Add new service to the server.
     *
     * @param service Service to be added
     */
    public void addService(Service service) {
        services.put(service.getServiceId(), service);
    }

    /**
     * Takes Codec with received data and process them.
     *
     * @param codec Codec with data
     */
    protected void processRequest(Codec codec) {
        MessageInfo info = codec.startReadMessage();

        if (info.type() != MessageType.kInvocationMessage && info.type() != MessageType.kOnewayMessage) {
            throw new RequestError("Invalid type of incoming request");
        }

        Service service = services.get(info.service());

        if (service == null) {
            throw new RequestError("Invalid service ID (" + info.service() + ")");
        }

        service.handleInvocation(info.request(), info.sequence(), codec);

        if (info.type() == MessageType.kOnewayMessage) {
            codec.reset();
        }
    }

    /**
     * Run the server.
     */
    public abstract void run();

    /**
     * Stop the server.
     */
    public abstract void stop();
}
