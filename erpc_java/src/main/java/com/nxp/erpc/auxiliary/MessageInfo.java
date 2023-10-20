/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

package com.nxp.erpc.auxiliary;

/**
 * Record storing information about message.
 *
 * @param type     Message type
 * @param service  Service identification
 * @param request  Request identification
 * @param sequence Sequence number
 */
public record MessageInfo(
        MessageType type,
        int service,
        int request,
        int sequence) {
}
