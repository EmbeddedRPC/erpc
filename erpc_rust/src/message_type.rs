/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#[derive(PartialEq, Debug, Clone, Copy)]
pub enum MessageType {
    Invocation = 0,
    Oneway = 1,
    Reply = 2,
    Notification = 3,
}

impl MessageType {
    pub fn from_u8(value: u8) -> MessageType {
        match value {
            0 => MessageType::Invocation,
            1 => MessageType::Oneway,
            2 => MessageType::Reply,
            3 => MessageType::Notification,
            _ => panic!("Invalid MessageType value: {}", value),
        }
    }
}
