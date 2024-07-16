/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use crate::codec::Codec;
use crate::transport::Transport;

pub trait Service<T: Codec> {
    fn service_id(&self) -> u8;

    fn handle_invocation(&mut self, method_id: u8, sequence: u32, codec: &mut T);
}

pub trait Server<T: Codec, S: Service<T>, R: Transport> {
    fn transport(&mut self) -> &mut R;

    fn set_transport(&mut self, transport: R);

    fn services(&self) -> &[S];

    fn add_service(&mut self, service: S);

    fn get_service_with_id(&mut self, service_id: u8) -> Option<&mut S>;

    fn process_request(&mut self) -> Result<(), String>;

    fn run(&mut self) -> Result<(), String>;

    fn stop(&mut self);
}
