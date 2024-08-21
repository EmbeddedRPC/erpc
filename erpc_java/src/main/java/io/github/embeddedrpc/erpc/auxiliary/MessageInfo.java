/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

package  io.github.embeddedrpc.erpc.auxiliary;

import java.util.Objects;

public final class MessageInfo {
        private final MessageType type;
        private final int service;
        private final int request;
        private final int sequence;

/**
 * Class storing information about message.
 *
 * @param type     Message type
 * @param service  Service identification
 * @param request  Request identification
 * @param sequence Sequence number
 */
  public MessageInfo(MessageType type, int service, int request, int sequence) {
     this.type = type;
     this.service = service;
     this.request = request;
     this.sequence = sequence;
  }

/**
 * Get message type.
 *
 * @return type
*/
  public MessageType type() {
      return type;
  }

/**
 * Get message service.
 *
 * @return service
*/
    public int service() {
        return service;
    }

/**
 * Get message request.
 *
 * @return request
*/
    public int request() {
        return request;
    }


/**
 * Get message sequence.
 *
 * @return sequence
*/
    public int sequence() {
        return sequence;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) {
            return true;
        }
        if (!(o instanceof MessageInfo)) {
            return false;
        }
        MessageInfo that = (MessageInfo) o;
        return service == that.service && request == that.request && sequence == that.sequence && type == that.type;
    }

    @Override
    public int hashCode() {
        return Objects.hash(type, service, request, sequence);
    }
}
