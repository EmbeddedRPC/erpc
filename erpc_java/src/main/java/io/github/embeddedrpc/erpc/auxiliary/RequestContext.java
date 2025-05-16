/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

package  io.github.embeddedrpc.erpc.auxiliary;

import  io.github.embeddedrpc.erpc.codec.Codec;
import groovyjarjarantlr.ByteBuffer;

import java.util.Objects;

public final class RequestContext {
  private final int sequence;
  private final ByteBuffer message;
  private final Codec codec;
  private final Boolean isOneWay;

/**
 * Class storing request context.
 *
 * @param sequence Sequence number
 * @param message  Message to be sent
 * @param codec    Codec used to store send and received message
 * @param isOneWay Request direction
 */
  public RequestContext(int sequence, ByteBuffer message, Codec codec, Boolean isOneWay) {
      this.sequence = sequence;
      this.message = message;
      this.codec = codec;
      this.isOneWay = isOneWay;
  }

/**
 * Get request context sequence.
 *
 * @return sequence
*/
    public int sequence() {
        return sequence;
    }

/**
 * Get request context message.
 *
 * @return message
*/
    public ByteBuffer message() {
        return message;
    }

/**
 * Get request context codec.
 *
 * @return codec
*/
    public Codec codec() {
        return codec;
    }

/**
 * is request context oneWay.
 *
 * @return isOneWay
*/
    public boolean isOneWay() {
        return isOneWay;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) {
            return true;
        }
        if (!(o instanceof RequestContext)) {
            return false;
        }
        RequestContext that = (RequestContext) o;
        return sequence == that.sequence
                && Objects.equals(message, that.message)
                && Objects.equals(codec, that.codec)
                && Objects.equals(isOneWay, that.isOneWay);
    }

    @Override
    public int hashCode() {
        return Objects.hash(sequence, message, codec, isOneWay);
    }
}
