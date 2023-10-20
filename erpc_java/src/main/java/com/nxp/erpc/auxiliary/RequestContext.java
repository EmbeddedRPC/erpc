/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

package com.nxp.erpc.auxiliary;

import com.nxp.erpc.codec.Codec;
import groovyjarjarantlr.ByteBuffer;

/**
 * Record storing request context.
 *
 * @param sequence Sequence number
 * @param message  Message to be sent
 * @param codec    Codec used to store send and received message
 * @param isOneWay Request direction
 */
public record RequestContext(int sequence, ByteBuffer message, Codec codec, Boolean isOneWay) {
}
