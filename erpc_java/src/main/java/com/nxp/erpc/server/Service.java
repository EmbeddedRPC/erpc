/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

package com.nxp.erpc.server;

import com.nxp.erpc.auxiliary.RequestError;
import com.nxp.erpc.codec.Codec;
import com.nxp.erpc.codec.CodecError;

import java.util.HashMap;
import java.util.Map;
import java.util.function.BiConsumer;

/**
 * Abstract class representing server's service. Service handle its methods invocation.
 */
public abstract class Service {
    private final int serviceId;
    private final Map<Integer, BiConsumer<Integer, Codec>> methodHandlers = new HashMap<>();

    /**
     * Create new service with ID.
     *
     * @param serviceId service ID
     */
    public Service(int serviceId) {
        this.serviceId = serviceId;
    }

    /**
     * Return service ID.
     *
     * @return ID
     */
    public int getServiceId() {
        return serviceId;
    }

    /**
     * Add method to the service.
     *
     * @param methodId method id
     * @param handler  function that handle method
     */
    protected void addMethodHandler(int methodId, BiConsumer<Integer, Codec> handler) {
        methodHandlers.put(methodId, handler);
    }

    /**
     * Handle service invocation. Find corresponding handler for method and execute it.
     *
     * @param methodId Method ID
     * @param sequence Message sequence number
     * @param codec    Codec with method's data
     */
    public void handleInvocation(int methodId, int sequence, Codec codec) {
        BiConsumer<Integer, Codec> method = methodHandlers.get(methodId);

        if (method == null) {
            throw new RequestError("Invalid method ID (" + methodId + ").");
        }

        try {
            method.accept(sequence, codec);
        } catch (CodecError | RequestError e) {
            throw new RequestError(
                    "Invalid method ID (" + methodId + ") or method implementation.", e);
        }
    }
}
