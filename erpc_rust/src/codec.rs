/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use crate::message_type::MessageType;

#[derive(Debug, Clone)]
pub struct MsgInfo {
    pub msg_type: MessageType,
    pub service: u8,
    pub request: u8,
    pub sequence: u32,
}

pub trait Codec {
    fn buffer(&self) -> &[u8];

    fn set_buffer(&mut self, buffer: Vec<u8>);

    fn reset(&mut self);

    fn start_write_message(&mut self, msg_info: MsgInfo);

    fn write_bool(&mut self, value: bool);

    fn write_i8(&mut self, value: i8);

    fn write_i16(&mut self, value: i16);

    fn write_i32(&mut self, value: i32);

    fn write_i64(&mut self, value: i64);

    fn write_u8(&mut self, value: u8);

    fn write_u16(&mut self, value: u16);

    fn write_u32(&mut self, value: u32);

    fn write_u64(&mut self, value: u64);

    fn write_f32(&mut self, value: f32);

    fn write_f64(&mut self, value: f64);

    fn write_string(&mut self, value: &str);

    fn write_binary(&mut self, value: &[u8]);

    fn start_write_list(&mut self, length: usize);

    fn start_write_union(&mut self, discriminator: u32);

    fn write_null_flag(&mut self, is_null: bool);

    fn start_read_message(&mut self) -> Result<MsgInfo, String>;

    fn read_bool(&mut self) -> bool;

    fn read_i8(&mut self) -> i8;

    fn read_i16(&mut self) -> i16;

    fn read_i32(&mut self) -> i32;

    fn read_i64(&mut self) -> i64;

    fn read_u8(&mut self) -> u8;

    fn read_u16(&mut self) -> u16;

    fn read_u32(&mut self) -> u32;

    fn read_u64(&mut self) -> u64;

    fn read_f32(&mut self) -> f32;

    fn read_f64(&mut self) -> f64;

    fn read_string(&mut self) -> String;

    fn read_binary(&mut self) -> Vec<u8>;

    fn start_read_list(&mut self) -> usize;

    fn start_read_union(&mut self) -> u32;

    fn read_null_flag(&mut self) -> bool;
}
