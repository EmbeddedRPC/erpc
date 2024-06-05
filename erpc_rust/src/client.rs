/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use crate::codec::{Codec, MsgInfo};
use crate::transport::Transport;

pub struct ClientManager<T: Codec + Clone, S: Transport> {
    pub transport: S,
    pub codec: T,
    sequence: u32,
}

impl<T: Codec + Clone, S: Transport> ClientManager<T, S> {
    pub fn new(transport: S, codec: T) -> Self {
        Self {
            transport,
            codec,
            sequence: 0,
        }
    }

    pub fn sequence(&mut self) -> u32 {
        self.sequence += 1;
        self.sequence
    }

    pub fn create_request(&mut self, is_oneway: bool) -> RequestContext<T> {
        let sequence = self.sequence();
        let message = vec![];
        let codec = self.codec.clone();
        RequestContext::new(sequence, message, codec, is_oneway)
    }

    pub fn perform_request(&mut self, request: &mut RequestContext<T>) -> Result<MsgInfo, String> {
        self.transport.send(request.codec.buffer())?;
        let response = self.transport.recv()?;
        request.codec.set_buffer(response);
        request.codec.start_read_message()
    }

    pub fn perform_oneway_request(&mut self, request: RequestContext<T>) -> Result<(), String> {
        self.transport.send(request.codec.buffer())
    }
}

pub struct RequestContext<T: Codec> {
    pub sequence: u32,
    pub message: Vec<u8>,
    pub codec: T,
    pub is_oneway: bool,
}

impl<'a, T: Codec> RequestContext<T> {
    pub fn new(sequence: u32, message: Vec<u8>, codec: T, is_oneway: bool) -> Self {
        Self {
            sequence,
            message,
            codec,
            is_oneway,
        }
    }

    pub fn sequence(&self) -> u32 {
        self.sequence
    }

    pub fn message(&self) -> &[u8] {
        &self.message
    }

    pub fn codec(&mut self) -> &mut T {
        &mut self.codec
    }

    pub fn is_oneway(&self) -> bool {
        self.is_oneway
    }
}
