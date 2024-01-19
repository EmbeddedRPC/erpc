/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

package  io.github.embeddedrpc.erpc.auxiliary;

/**
 * Message type.
 */
public enum MessageType {
    /**
     * Invocation message.
     */
    kInvocationMessage(0),

    /**
     * One way message.
     */
    kOnewayMessage(1),

    /**
     * Reply message.
     */
    kReplyMessage(2),

    /**
     * Notification message.
     */
    kNotificationMessage(3);

    private final int value;

    /**
     * MessageType constructor.
     *
     * @param value Value to be assigned
     */
    MessageType(int value) {
        this.value = value;
    }

    /**
     * Get message type value.
     *
     * @return message value
     */
    public int getValue() {
        return value;
    }

    /**
     * Function returns @{MessageType} base on value.
     *
     * @param value Value to be converted to MessageType
     * @return Corresponding MessageType
     */
    public static MessageType getMessageType(int value) {
        return switch (value) {
            case 0 -> kInvocationMessage;
            case 1 -> kOnewayMessage;
            case 2 -> kReplyMessage;
            case 3 -> kNotificationMessage;
            default -> throw new RuntimeException("Invalid MessageType " + value);
        };
    }
}
